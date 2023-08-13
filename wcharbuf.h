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

#include "common.h"
#include "wstring.h"

typedef struct _C_Wchar_Buffer  C_Wchar_Buffer;

FMC_API void C_Wchar_Buffer_new(C_Wchar_Buffer* *newref);

FMC_API void C_Wchar_Buffer_new_from_string(C_Wchar_Buffer* *newref, C_Wstring* str);

FMC_API size_t C_Wchar_Buffer_length(C_Wchar_Buffer* self);

FMC_API wchar_t C_Wchar_Buffer_get_char(C_Wchar_Buffer* self, int index);

FMC_API bool C_Wchar_Buffer_get_slice(C_Wchar_Buffer* self, int first, int last, C_Wstring* *sp);


FMC_API void C_Wchar_Buffer_append_char(C_Wchar_Buffer* self, wchar_t c);

FMC_API void C_Wchar_Buffer_append_string(C_Wchar_Buffer* self, C_Wstring* s);

FMC_API void C_Wchar_Buffer_insert_char(C_Wchar_Buffer* self, int index, wchar_t c);

FMC_API void C_Wchar_Buffer_insert_string(C_Wchar_Buffer* self, int index, C_Wstring* s);

FMC_API void C_Wchar_Buffer_set_char(C_Wchar_Buffer* self, int index, wchar_t c);

FMC_API void C_Wchar_Buffer_set_slice(C_Wchar_Buffer* self, int first, int last, C_Wstring* str);

FMC_API void C_Wchar_Buffer_set_string(C_Wchar_Buffer* self, int index, C_Wstring* s);

FMC_API bool C_Wchar_Buffer_to_string(C_Wchar_Buffer* self, C_Wstring* *sp);

FMC_API bool C_Wchar_Buffer_is_live(const C_Wchar_Buffer* b);

FMC_API C_Wchar_Buffer* C_Wchar_Buffer_retain(const C_Wchar_Buffer* b);

FMC_API bool C_Wchar_Buffer_release(const C_Wchar_Buffer* *bptr);

FMC_API void C_Wchar_Buffer_set(const C_Wchar_Buffer* *lvalue, const C_Wchar_Buffer* rvalue);

#endif // FMC_WCHARBUF_H_INCLUDED

