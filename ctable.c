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
#define TABLE_LOAD      0.75

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
    if (a->len != b->len) return false;

    if (a->len == 0) return a->ptr == b->ptr;

    return bcmp(a->ptr, b->ptr, a->len) == 0 ? true : false;
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

struct C_Table {

    C_Table_Entry* *array;
    size_t         arraylen;
    size_t         nentries;

    C_Table_Hash    hash;
    C_Userdata_Equals   eq;
    C_Userdata_Copy cp;
    C_Userdata_Free rm;
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

static uint64_t hashcode(C_Table* t, const C_Userdata* key) {
    if (C_Userdata_is_reference(key)) {
        return (uint64_t)key->ptr;
    } else {
        return t->hash(key->ptr, key->len);
    }
}

static bool udequals(C_Table* t, const C_Userdata* a, const C_Userdata* b) {
    if (a == b) return true;

    if (!a || !b) return false;

    if (a->tag == DEFAULT_TAG && b->tag == DEFAULT_TAG
            && a->len == 0 && b->len == 0) {
        return a->ptr == b->ptr;
    }

    return t->eq(a, b);
}

static bool udcopy(C_Table* t, C_Userdata* to, const C_Userdata* from) {
    if (from->len == 0 || from->ptr == NULL) {
        to->tag = from->tag;
        to->len = 0;
        to->ptr = from->ptr;
        return true;
    } else {
        return t->cp(to, from);
    }
}

static void udfree(C_Table* t, C_Userdata* ud) {
    if (ud->len == 0 || ud->ptr == NULL) {
        ud->tag = 0;
        ud->len = 0;
        ud->ptr = NULL;
    } else {
        t->rm(ud);
    }
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
            udfree(t, &(prev->key));
            udfree(t, &(prev->value));

            free(prev);
        }
    }

    free(t->array);
    free(t);
    tptr = NULL;
}

static void insert_entry(C_Table* t, C_Table_Entry* entry) {
    size_t index = hashcode(t, &(entry->key)) % t->arraylen;

    entry->next = t->array[index];
    t->array[index] = entry;
}

static void rehash(C_Table* t) {
    C_Table_Entry* head = NULL;
    C_Table_Entry* tail = NULL;

    // First gather up all the entries in a big chain
    for (int i = 0; i < t->arraylen; i++) {
        C_Table_Entry* curr = t->array[i];

        if (curr == NULL) continue;

        if (head == NULL) {
            head = curr;
        }
        if (tail != NULL) {
            tail->next = curr;
        }
        tail = curr;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        t->array[i] = NULL;
    }

    // Then replace them using the new functions / array size
    while (head != NULL) {
        C_Table_Entry* prev = head;

        head = head->next;
        prev->next = NULL;
        insert_entry(t, prev);
    }
}

/* --------------------- Configuration Functions -------------------------*/

extern size_t C_Table_size(C_Table* t) {
    return t->nentries;
}

extern void C_Table_define_hash_function(C_Table* t, C_Table_Hash f) {
    if (t == NULL) return;
    if (f == NULL) {
        t->hash = default_hash;
    } else {
        t->hash = f;
    }
    rehash(t);
}

extern void C_Table_define_data_equals(C_Table* t, C_Userdata_Equals f) {
    if (t == NULL) return;
    if (f == NULL) {
        t->eq = default_equals;
    } else {
        t->eq = f;
    }
    rehash(t);
}

extern void C_Table_define_data_copy(C_Table* t, C_Userdata_Copy f) {
    if (t == NULL) return;
    if (f == NULL) {
        t->cp = default_copy;
    } else {
        t->cp = f;
    }
    rehash(t);
}

extern void C_Table_define_data_free(C_Table* t, C_Userdata_Free f) {
    if (t == NULL) return;
    if (f == NULL) {
        t->rm = default_free;
    } else {
        t->rm = f;
    }
    rehash(t);
}

/* ---------------------- Table Entry Functions --------------------------*/


static C_Table_Entry* find_entry(C_Table* t, const C_Userdata* key, C_Table_Entry* (*prevptr)) {
    C_Table_Entry* result = NULL;
    C_Table_Entry* prev = NULL;
    uint64_t h = hashcode(t, key);

    result = t->array[h % t->arraylen];
    while (result != NULL && !udequals(t, &(result->key), key)) {
        prev = result;
        result = result->next;
    }
    if (prevptr) {
        (*prevptr) = prev;
    }
    return result;
}

static bool insert_pair(C_Table* t, const C_Userdata* key, const C_Userdata* value) {
    C_Table_Entry* entry = (C_Table_Entry*)malloc(sizeof(C_Table_Entry));

    udcopy(t, &(entry->key), key);
    udcopy(t, &(entry->value), value);

    insert_entry(t, entry);

    t->nentries++;

    if (t->nentries >= TABLE_LOAD * t->arraylen) {
        size_t oldlen = t->arraylen;
        size_t newlen = oldlen * 2 + 1;
        C_Table_Entry** newarray 
            = reallocarray(t->array, newlen, sizeof(C_Table_Entry*));
        if (newarray != NULL) {
            for (int i = oldlen; i < newlen; i++) {
                // TODO: Yes, there's a better way to do this
                newarray[i] = NULL;
            }
            // bzero(&(newarray[oldlen]), (newlen - oldlen) * sizeof(C_Table_Entry*));
            t->array = newarray;
            t->arraylen = newlen;
            rehash(t);
        }
    }

    return true;
}

static bool update_entry(C_Table* t, C_Table_Entry* entry, const C_Userdata* value) {
    C_Userdata* entval = &(entry->value);
    C_Userdata newval;

    udcopy(t, &newval, value);
    if (!(udequals(t, &newval, value))) {
        udfree(t, &newval);
        return false;
    }
    udfree(t, entval);

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

    return insert_pair(t, key, value);
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
        return insert_pair(t, key, value);
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

    udfree(t, &(entry->key));
    udfree(t, &(entry->value));

    return true;
}

/* -------------------- Iterator Functions ------------------------- */

struct C_Table_Iterator {
    C_Table_Entry** entryset;
    int             pos;
    int             len;
};

extern void C_Table_new_iterator(C_Table* t, C_Table_Iterator* *iptr) {
    C_Table_Iterator* result;
    C_Table_Entry**   resultarr;
    int               j, jmax;

    if (t == NULL || iptr == NULL) return;

    result = (C_Table_Iterator*)malloc(sizeof(C_Table_Iterator));
    if (result == NULL) return;

    // Add empty slots before and after entries for before iteration
    // starts and after it stops, lest we run off the array either way.
    jmax = t->nentries + 1;

    resultarr = (C_Table_Entry**)calloc(jmax + 1, sizeof(C_Table_Entry*));
    if (result == NULL) {
        free(result);
        return;
    }

    j = 1;
    for (int i = 0; i < t->arraylen && j < jmax; i++) {
        C_Table_Entry* curr = t->array[i];

        while (curr != NULL) {
            resultarr[j] = curr;
            j++;
            curr = curr->next;
        }
    }
    result->entryset = resultarr;
    result->pos = 0;
    result->len = jmax;

    *iptr = result;
}

extern bool C_Table_Iterator_has_next(C_Table_Iterator* i) {
    return i->pos < i->len - 1;
}

extern void C_Table_Iterator_next(C_Table_Iterator* i) {
    i->pos++;
}

static C_Table_Entry* get_entry(C_Table_Iterator* i) {
    if (i->pos >= 0 && i->pos < i->len) {
        return i->entryset[i->pos];
    }
    return NULL;
}

extern bool C_Table_Iterator_current_key(C_Table_Iterator* i, C_Userdata *key) {
    C_Table_Entry* entry = get_entry(i);
    if (entry == NULL) return false;
    C_Userdata_set(key, entry->key.tag, entry->key.len, entry->key.ptr);
    return true;
}

extern bool C_Table_Iterator_current_pair(C_Table_Iterator* i, C_Userdata *key, C_Userdata *value) {
    C_Table_Entry* entry = get_entry(i);
    if (entry == NULL) return false;
    C_Userdata_set(key, entry->key.tag, entry->key.len, entry->key.ptr);
    C_Userdata_set(value, entry->value.tag, entry->value.len, entry->value.ptr);
    return true;
}

extern bool C_Table_Iterator_free(C_Table_Iterator* *iptr) {
    if (!(iptr)) return false;

    free((*iptr)->entryset);
    free(*iptr);
    *iptr = NULL;
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

extern void C_Userdata_set_value(C_Userdata* ud, const void* ptr, size_t len) {
    ud->tag = DEFAULT_TAG;
    ud->len = len;
    ud->ptr = (void*)ptr;
}
