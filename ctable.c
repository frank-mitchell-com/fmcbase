/*
 * Copyright 2023 Frank Mitchell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "ctable.h"

#define TABLE_MINSIZ    5

typedef struct _C_Table_Entry C_Table_Entry;

struct _C_Table_Entry {
    C_Table_Entry* next;

    C_Userdata key;
    C_Userdata value;
};

static uint64_t default_hash(const void* ptr, const size_t len) {
    // Treat `ptr` as a list of bytes
    const uint8_t* bytes = (const uint8_t*)ptr;
    // Stolen from:
    // https://www.geeksforgeeks.org/string-hashing-using-polynomial-rolling-hash-function/
    int p = 31, m = 1000000007;
    long p_pow = 1;
    uint64_t hash_so_far = 0;

    for (int i = 0; i < len; i++) {
        hash_so_far += (bytes[i] * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return hash_so_far;
}

static bool default_equals(const C_Userdata* a, const C_Userdata* b) {
    unsigned char* adata;
    unsigned char* bdata;

    if (a == b) return true;

    if (!a || !b) return false;

    if (a->len != b->len) return false;

    adata = (unsigned char*)a->ptr;
    bdata = (unsigned char*)b->ptr;
    for (int i = 0; i < a->len; i++) {
        if (adata[i] != bdata[i]) {
            return false;
        }
    }
    return true;
}

static bool default_copy(C_Userdata* to, const C_Userdata* from) {
    void* ptr;

    if (!to) return false;

    ptr = malloc(1 + from->len);
    if (!ptr) return false;
    memcpy(ptr, from->ptr, from->len);

    C_Userdata_set(to, from->tag, from->len, ptr);
    return true;
}

static void default_free(C_Userdata* ud) {
    if (!ud) return;
    free(ud->ptr);
}

struct _C_Table {

    C_Table_Entry* *array;
    size_t         arraylen;
    size_t         nentries;

    C_Table_Hash    hash;
    C_Userdata_Eq   eq;
    C_Userdata_Copy cp;
    C_Userdata_Free rm;
};

struct _C_Table_Iterator {
};

extern void C_Table_new(C_Table* *tptr, size_t minsz) {
    C_Table* t;
    if (!tptr) return;
    (*tptr) = NULL;

    t = malloc(sizeof(C_Table));
    if (!t) return;

    bzero(t, sizeof(C_Table));
    t->hash     = default_hash;
    t->eq       = default_equals;
    t->cp       = default_copy;
    t->rm       = default_free;
    t->arraylen = (minsz > TABLE_MINSIZ) ? minsz : TABLE_MINSIZ;
    t->array    = calloc(t->arraylen, sizeof(C_Table_Entry*));
    t->nentries = 0;

    (*tptr) = t;
}

extern void C_Table_free(C_Table* *tptr) {
    C_Table* t;

    if (!tptr) return;
    t = *tptr;

    // free all entries in array
    for (int i = 0; i < t->arraylen; i++) {
        C_Table_Entry* head = t->array[i];
        t->array[i] = NULL;
        while (head != NULL) {
            C_Table_Entry* prev = head;

            head = head->next;

            // free all keys and values
            t->rm(&(prev->key));
            t->rm(&(prev->value));

            free(prev);
        }
    }

    free(t->array);
    free(t);
    tptr = NULL;
}

/* ---------------------- Table Entry Functions --------------------------*/


static uint64_t hashcode(C_Table* t, const C_Userdata* key) {
    if (C_Userdata_is_reference(key)) {
        return (uint64_t)key->ptr;
    } else {
        return t->hash(key->ptr, key->len);
    }
}

static C_Table_Entry* find_entry(C_Table* t, const C_Userdata* key, C_Table_Entry* (*prevptr)) {
    C_Table_Entry* result = NULL;
    C_Table_Entry* prev = NULL;
    uint64_t h = hashcode(t, key);

    result = t->array[h % t->arraylen];
    while (result != NULL && !t->eq(&(result->key), key)) {
        prev = result;
        result = result->next;
    }
    if (prevptr) {
        (*prevptr) = prev;
    }
    return result;
}

static bool insert_entry(C_Table* t, const C_Userdata* key, const C_Userdata* value) {
    size_t index = hashcode(t, key) % t->arraylen;

    C_Table_Entry* entry = (C_Table_Entry*)malloc(sizeof(C_Table_Entry));

    t->cp(&(entry->key), key);
    t->cp(&(entry->value), value);

    entry->next = t->array[index];
    t->array[index] = entry;

    return true;
}

static bool update_entry(C_Table* t, C_Table_Entry* entry, const C_Userdata* value) {
    C_Userdata* entval = &(entry->value);
    C_Userdata newval;

    t->cp(&newval, value);
    if (!(t->eq(&newval, value))) {
        t->rm(&newval);
        return false;
    }
    t->rm(entval);

    //C_Userdata_set(entval, newval.tag, newval.len, newval.ptr);
    entry->value.tag = newval.tag;
    entry->value.len = newval.len;
    entry->value.ptr = newval.ptr;
    return true;
}

extern bool C_Table_add(C_Table* t, const C_Userdata* key, const C_Userdata* value) {
    C_Table_Entry* entry;

    if (!key || !value) return false;

    entry = find_entry(t, key, NULL);
    if (entry != NULL) {
        return false;
    }

    return insert_entry(t, key, value);
}

extern bool C_Table_get(C_Table* t, const C_Userdata* key, C_Userdata* value) {
    C_Table_Entry* entry;

    if (!key || !value) return false;

    entry = find_entry(t, key, NULL);
    if (entry == NULL) {
        return false;
    }
    C_Userdata_set(value, entry->value.tag, entry->value.len, entry->value.ptr);
    return true;
}

extern bool C_Table_has(C_Table* t, const C_Userdata* key) {
    if (!key) return false;

    return find_entry(t, key, NULL) != NULL;
}

extern bool C_Table_put(C_Table* t, const C_Userdata* key, const C_Userdata* value) {
    C_Table_Entry* entry;

    if (!key || !value) return false;

    entry = find_entry(t, key, NULL);
    if (entry == NULL) {
        return insert_entry(t, key, value);
    }

    return update_entry(t, entry, value);
}

extern bool C_Table_remove(C_Table* t, const C_Userdata* key) {
    C_Table_Entry* entry;
    C_Table_Entry* prev;

    if (!key) return false;

    entry = find_entry(t, key, &prev);
    if (entry == NULL) {
        return false;
    }

    if (prev != NULL) {
        prev->next = entry->next;
    } else {
        size_t index = hashcode(t, key) % t->arraylen;

        t->array[index] = entry->next;
    }

    t->rm(&(entry->key));
    t->rm(&(entry->value));

    return true;
}


/* -------------------- Userdata Functions ------------------------- */

extern bool C_Userdata_is_reference(const C_Userdata* ud) {
    return ud && (ud->len == 0);
}

extern void C_Userdata_clear(C_Userdata* ud, bool iscopy) {
    if (!ud) return;

    if (iscopy && !C_Userdata_is_reference(ud)) {
        // TODO: What if ud isn't a copy?
        free(ud->ptr);
    }
    bzero(ud, sizeof(C_Userdata));
}

extern void C_Userdata_set(C_Userdata* ud, tag_t tag, size_t len, const void* ptr) {
    if (!ud) return;
    ud->tag = tag;
    ud->len = len;
    ud->ptr = (void*)ptr;
}

extern void C_Userdata_set_string(C_Userdata* ud, const char* cstring) {
    ud->tag = DEFAULT_TAG;
    ud->len = strlen(cstring);
    ud->ptr = (void*)cstring;
}

extern void C_Userdata_set_pointer(C_Userdata* ud, const void* ref) {
    ud->tag = DEFAULT_TAG;
    ud->len = 0;
    ud->ptr = (void*)ref;
}

void C_Userdata_set_value(C_Userdata* ud, const void* ptr, size_t len) {
    ud->tag = DEFAULT_TAG;
    ud->len = len;
    ud->ptr = (void*)ptr;
}

