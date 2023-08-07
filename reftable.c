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
#include "reftable.h"

#define TBLMINSIZ   5

typedef struct ref_pair {
    const void *key;
    const void *value;
} C_Ref_Pair;

struct C_Ref_Table {
    C_Ref_Pair* data;
    size_t      len;
    size_t      npairs;
};

extern void C_Ref_Table_new(C_Ref_Table* *tptr, size_t minsz) {
    C_Ref_Table* self;
    C_Ref_Pair*  data;
    size_t       len;

    if (!tptr) {
        return;
    }
    *tptr = NULL;

    self = malloc(sizeof(C_Ref_Table));
    if (!self) {
        return;
    }

    len  = (minsz > TBLMINSIZ) ? minsz : TBLMINSIZ;
    data = calloc(len, sizeof(C_Ref_Pair));
    if (data == NULL) {
        free(self);
        return;
    }
    self->data   = data;
    self->len    = len;
    self->npairs = 0;

    *tptr = self;
}

static uint64_t hashcode(const void* k) {
    return ((uintptr_t)k) >> 2;
}

static ssize_t find_key(C_Ref_Pair data[], size_t len, const void* k, bool search) {
    ssize_t result = hashcode(k) % len;

    if (data[result].key == k) {
        return result;
    } else if (data[result].key == NULL) {
        return search ? -1 : result;
    } else {
        const void* target = search ? k : NULL;

        for (ssize_t i = (result+1) % len; i != result; i = (i+1) % len) {
            if (data[i].key == target) {
                return i;
            }
        }
        return -1;
    }
}

extern size_t C_Ref_Table_size(C_Ref_Table* self) {
    return self->npairs;
}

extern const void* C_Ref_Table_get(C_Ref_Table* self, const void* k) {
    ssize_t index = find_key(self->data, self->len, k, true);
    if (index < 0) {
        return NULL;
    }
    return self->data[index].value;
}

extern bool C_Ref_Table_has(C_Ref_Table* self, const void* k) {
    return find_key(self->data, self->len, k, true) >= 0;
}

static bool rehash(C_Ref_Table* self) {
    size_t      oldlen = self->len;
    C_Ref_Pair* olddata = self->data;
    size_t      newlen = oldlen * 2 + 1;
    C_Ref_Pair* newdata = calloc(newlen, sizeof(C_Ref_Pair));

    if (newdata == NULL) {
        return false;
    }
    for (size_t i = 0; i < oldlen; i++) {
        C_Ref_Pair* curr = olddata + i;
        if (curr->key != NULL) {
            ssize_t j = find_key(newdata, newlen, curr->key, false);
            if (j >= 0) {
                newdata[j].key = curr->key;
                newdata[j].value = curr->value;
            } else {
                // shouldn't happen ... what do we do?
                free(newdata);
                return false;
            }
        }
    }
    free(olddata);
    self->data = newdata;
    self->len  = newlen;
    return true;
}

extern bool C_Ref_Table_put(C_Ref_Table* self, const void* k, const void* v, const void* *oldvalp) {
    ssize_t index;

    if (self->npairs > self->len * 0.75) {
        if (!rehash(self)) {
            return false;
        }
    }

    index = find_key(self->data, self->len, k, false);

    if (index < 0) {
        return false;
    }

    if (oldvalp) {
        (*oldvalp) = self->data[index].value;
    }

    if (self->data[index].key == NULL) {
        self->npairs++;
    }
    self->data[index].key = k;
    self->data[index].value = v;
    return true;
}

extern bool C_Ref_Table_remove(C_Ref_Table* self, const void* k, const void* *oldvalp) {
    ssize_t index = find_key(self->data, self->len, k, true);
    if (index < 0) {
        return false;
    }

    if (oldvalp) {
        (*oldvalp) = self->data[index].value;
    }

    self->npairs--;
    self->data[index].key = NULL;
    self->data[index].value = NULL;
    return true;
}

void C_Ref_Table_free(C_Ref_Table* *selfptr) {
    C_Ref_Table* self = selfptr ? *selfptr : NULL;
    if (!self) {
        return;
    }
    free(self->data);
    free(self);
    *selfptr = NULL;
}

