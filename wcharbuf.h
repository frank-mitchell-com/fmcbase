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

#ifndef FMC_WCHARBUF_H_INCLUDED
#define FMC_WCHARBUF_H_INCLUDED

#include "wstring.h"

#define USTR_API    extern


typedef struct _U_Char_Buffer*  U_Char_Buffer;

USTR_API void U_Char_Buffer_new(U_Char_Buffer* *newref);

USTR_API void U_Char_Buffer_new_from_string(U_Char_Buffer* *newref, U_String* str);

USTR_API size_t U_Char_Buffer_length(U_Char_Buffer* self);

USTR_API wchar_t U_Char_Buffer_get_char(U_Char_Buffer* self, int index);

USTR_API bool U_Char_Buffer_get_slice(U_Char_Buffer* self, int first, int last, U_String* *sp);


USTR_API void U_Char_Buffer_append_char(U_Char_Buffer* self, wchar_t c);

USTR_API void U_Char_Buffer_append_string(U_Char_Buffer* self, U_String* s);

USTR_API void U_Char_Buffer_insert_char(U_Char_Buffer* self, int index, wchar_t c);

USTR_API void U_Char_Buffer_insert_string(U_Char_Buffer* self, int index, U_String* s);

USTR_API void U_Char_Buffer_set_char(U_Char_Buffer* self, int index, wchar_t c);

USTR_API void U_Char_Buffer_set_slice(U_Char_Buffer* self, int first, int last, U_String* str);

USTR_API void U_Char_Buffer_set_string(U_Char_Buffer* self, int index, U_String* s);

USTR_API bool U_Char_Buffer_to_string(U_Char_Buffer* self, U_String* *sp);

USTR_API bool U_Char_Buffer_is_live(U_Char_Buffer* b);

USTR_API void U_Char_Buffer_retain(U_Char_Buffer* *bptr);

USTR_API void U_Char_Buffer_release(U_Char_Buffer* *bptr);

USTR_API void U_Char_Buffer_set(U_Char_Buffer* *lvalue, U_Char_Buffer* rvalue);

#endif // FMC_WCHARBUF_H_INCLUDED

