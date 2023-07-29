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

#ifndef USTRARR_H_INCLUDED
#define USTRARR_H_INCLUDED

#include "ustring.h"

typedef struct _U_String_Array* U_String_Array;

USTR_API void U_String_Array_new(U_String_Array* *newref);

USTR_API size_t U_String_Array_size(U_String_Array* self);

USTR_API ustring_t U_String_Array_get_string(U_String_Array* self, int index);

USTR_API void U_String_Array_append_string(U_String_Array* self, ustring_t s);

USTR_API void U_String_Array_insert_string(U_String_Array* self, int index, ustring_t s);

USTR_API void U_String_Array_set_string(U_String_Array* self, int index, ustring_t s);

USTR_API bool U_String_Array_is_live(U_String_Array* a);

USTR_API void U_String_Array_retain(U_String_Array* *aptr);

USTR_API void U_String_Array_release(U_String_Array* *aptr);

USTR_API void U_String_Array_set(U_String_Array* *lvalue, U_String_Array* rvalue);

#endif // USTRARR_H_INCLUDED
