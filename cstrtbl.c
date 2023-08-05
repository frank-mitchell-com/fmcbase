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
#include "ctable.h"
#include "cstrtbl.h"

struct C_String_Table {
    C_Table* t;
};

extern void C_String_Table_new(C_String_Table* *tptr, size_t minsz) {
    C_Table* t;
    C_String_Table* self;

    if (!tptr) {
        return;
    }
    *tptr = NULL;

    self = malloc(sizeof(C_String_Table));
    if (!self) {
        return;
    }
    C_Table_new(&t, minsz);
    if (t == NULL) {
        free(self);
        return;
    }
    self->t = t;

    *tptr = self;
}

extern size_t C_String_Table_size(C_String_Table* t) {
    return C_Table_size(t->t);
}

extern const void* C_String_Table_get(C_String_Table* t, size_t kl, const uint8_t* kp) {
    C_Userdata key, value;

    C_Userdata_set_value(&key, kp, kl);
    C_Userdata_clear(&value, false);

    C_Table_get(t->t, &key, &value);

    return value.ptr;
}

extern bool C_String_Table_has(C_String_Table* t, size_t kl, const uint8_t* kp) {
    C_Userdata key;

    C_Userdata_set_value(&key, kp, kl);

    return C_Table_has(t->t, &key);
}

extern bool C_String_Table_add(C_String_Table* t, size_t kl, const uint8_t* kp, const void* v) {
    C_Userdata key, value;

    C_Userdata_set_value(&key, kp, kl);
    C_Userdata_set_pointer(&value, v);
    return C_Table_add(t->t, &key, &value);
}

extern bool C_String_Table_remove(C_String_Table* t, size_t kl, const uint8_t* kp, const void* *oldvalp) {
    C_Userdata key, value;

    C_Userdata_set_value(&key, kp, kl);

    if (oldvalp) {
        C_Userdata_clear(&value, false);
        C_Table_get(t->t, &key, &value);
        *oldvalp = value.ptr;
    }
    return C_Table_remove(t->t, &key);
}

void C_String_Table_free(C_String_Table* *tptr) {
    C_Table* t;
    if (!tptr) {
        return;
    }
    t = (*tptr)->t;
    C_Table_free(&t);
    free(*tptr);
    *tptr = NULL;
}

