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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "refset.h"

#define TABLE_MINSIZ    5
#define TABLE_LOAD      0.75

struct C_Ref_Set {
    const void*  *array;
    size_t       arraylen;
    size_t       nentries;
};


FMC_API void C_Ref_Set_new(C_Ref_Set* *rsptr, size_t minsz) {
    C_Ref_Set* rs;
    if (!rsptr) return;
    (*rsptr) = NULL;

    rs = malloc(sizeof(C_Ref_Set));
    if (!rs) return;

    bzero(rs, sizeof(C_Ref_Set));
    rs->arraylen = (minsz > TABLE_MINSIZ) ? minsz : TABLE_MINSIZ;
    rs->array    = calloc(rs->arraylen, sizeof(void*));
    rs->nentries = 0;

    (*rsptr) = rs;
}

FMC_API void C_Ref_Set_free(C_Ref_Set* *rsptr) {
    C_Ref_Set* rs;

    if (!rsptr) return;
    rs = *rsptr;

    free(rs->array);
    free(rs);
    rsptr = NULL;
}

FMC_API size_t C_Ref_Set_size(C_Ref_Set* rs) {
    return rs->nentries;
}

/* ---------------------- Table Entry Functions --------------------------*/

static uint64_t hashcode(const void* p) {
    // The lowest two bits are 0 on >= 4 byte architectures.
    return ((uintptr_t)p) >> 2;
}

static int search(const void* *array, size_t len, const void* target, int start) {
    // Another pointer in its place; check the next most likely places
    // TODO: Implement better open address algorithm
    int index = (start + 1) % len;
    while (index != start && array[index] != target) {
        index = (index + 1) % len;
    }
    if (index == start) {
        // Searched the whole array back to where we started, 
        // found nothing.
        return -1;
    } else {
        // Found it!
        return index;
    }
}

static int find_entry(C_Ref_Set* rs, const void* p) {
    const void** array = rs->array;
    size_t len = rs->arraylen;
    int result = hashcode(p) % len;

    if (array[result] == p) {
        return result;
    } else if (array[result] == NULL) {
        return -1;
    } else {
        return search(array, len, p, result);
    }
}

static const void** rehash(const void* *oldarray, size_t oldlen, size_t newlen) {
    const void** newarray = calloc(newlen, sizeof(void*));

    for (size_t i = 0; i < oldlen; i++) {
        const void* p = oldarray[i];
        int index = hashcode(p) % newlen;

        if (newarray[index] != NULL) {
            index = search(newarray, newlen, NULL, index);
        }
        newarray[index] = p;
    }
    return newarray;
}

static bool insert_entry(C_Ref_Set* rs, const void* p) {
    int index;

    // Make the array bigger if warranted
    if (rs->nentries >= TABLE_LOAD * rs->arraylen) {
        size_t oldlen = rs->arraylen;
        size_t newlen = oldlen * 2 + 1;
        const void** newarray = rehash(rs->array, oldlen, newlen);

        if (newarray != NULL) {
            rs->array = newarray;
            rs->arraylen = newlen;
        }
    }

    index = hashcode(p) % rs->arraylen;

    if (rs->array[index] != NULL) {
        index = search(rs->array, rs->arraylen, NULL, index); 
        if (index < 0) {
            return false;
        }
    }

    if (rs->array[index] == NULL) {
        rs->nentries++;
    }
    rs->array[index] = p;

    return true;
}

FMC_API bool C_Ref_Set_add(C_Ref_Set* rs, const void* p) {
    int index = -1;

    if (!p) return false;

    index = find_entry(rs, p);
    if (index < 0) {
        bool result = insert_entry(rs, p);
        return result;
    }
    return false;
}

FMC_API bool C_Ref_Set_has(C_Ref_Set* rs, const void* p) {
    if (!p) return false;

    return find_entry(rs, p) >= 0;
}

FMC_API bool C_Ref_Set_remove(C_Ref_Set* rs, const void* p) {
    int index = -1;

    if (!p) return false;

    index = find_entry(rs, p);
    if (index >= 0) {
        rs->array[index] = NULL;
        rs->nentries--;
        return true;
    }
    return false;
}

/* -------------------- Iterator Functions ------------------------- */

struct C_Ref_Set_Iterator {
    C_Ref_Set* rs;
    int64_t    curr;
    int64_t    next;
};

FMC_API void C_Ref_Set_new_iterator(C_Ref_Set* rs, C_Ref_Set_Iterator* *iptr) {
    C_Ref_Set_Iterator* result;

    result = malloc(sizeof(C_Ref_Set_Iterator));

    result->rs    = rs;
    result->curr = -1;
    result->next = -1;

    for (size_t j = 0; j < rs->arraylen; j++) {
        if (rs->array[j] != NULL) {
            result->next = j;
            break;
        }
    }

    *iptr = result;
}

FMC_API bool C_Ref_Set_Iterator_has_next(C_Ref_Set_Iterator* i) {
    return i->next >= 0;
}

FMC_API void C_Ref_Set_Iterator_next(C_Ref_Set_Iterator* i) {
    size_t len = i->rs->arraylen;
    const void** arr = i->rs->array;

    i->curr = i->next;

    for (int j = i->next+1; j < len; j++) {
        if (arr[j] != NULL) {
            i->next = j;
            break;
        }
    }
    if (i->curr == i->next) {
        i->next = -1;
    }
}

FMC_API const void* C_Ref_Set_Iterator_current(C_Ref_Set_Iterator* i) {
    if (i->curr < 0 || i->curr >= i->rs->arraylen) return NULL;
    return i->rs->array[i->curr];
}

FMC_API bool C_Ref_Set_Iterator_free(C_Ref_Set_Iterator* *iptr) {
    if (!(iptr)) return false;

    free(*iptr);
    *iptr = NULL;
    return true;
}


