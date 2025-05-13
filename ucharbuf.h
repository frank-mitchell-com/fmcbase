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

#ifndef FMC_UCHARBUF_H_INCLUDED
#define FMC_UCHARBUF_H_INCLUDED

#include "common.h"
#include "ustring.h"

typedef struct _C_Uchar_Buffer  C_Uchar_Buffer;

FMC_API void C_Uchar_Buffer_new(C_Uchar_Buffer* *newref);

FMC_API void C_Uchar_Buffer_new_size(C_Uchar_Buffer* *newref, size_t capacity);

FMC_API void C_Uchar_Buffer_new_from_ucs(C_Uchar_Buffer* *newref, const char32_t* ucs);

FMC_API void C_Uchar_Buffer_new_from_string(C_Uchar_Buffer* *newref, const C_Ustring* str);

FMC_API size_t C_Uchar_Buffer_length(C_Uchar_Buffer* self);

FMC_API char32_t C_Uchar_Buffer_char_at(C_Uchar_Buffer* self, ssize_t index);

FMC_API bool C_Uchar_Buffer_slice(C_Uchar_Buffer* self, ssize_t first, ssize_t last, const C_Ustring* *sp);


FMC_API void C_Uchar_Buffer_append_char(C_Uchar_Buffer* self, char32_t c);

FMC_API void C_Uchar_Buffer_append_string(C_Uchar_Buffer* self, const C_Ustring* s);

FMC_API void C_Uchar_Buffer_insert_char(C_Uchar_Buffer* self, ssize_t index, char32_t c);

FMC_API void C_Uchar_Buffer_insert_string(C_Uchar_Buffer* self, ssize_t index, const C_Ustring* s);

FMC_API void C_Uchar_Buffer_set_char(C_Uchar_Buffer* self, ssize_t index, char32_t c);

FMC_API void C_Uchar_Buffer_set_slice(C_Uchar_Buffer* self, ssize_t first, ssize_t last, const C_Ustring* str);

FMC_API void C_Uchar_Buffer_set_string(C_Uchar_Buffer* self, ssize_t index, const C_Ustring* s);

FMC_API bool C_Uchar_Buffer_to_string(C_Uchar_Buffer* self, const C_Ustring* *sp);

FMC_API bool C_Uchar_Buffer_is_live(const C_Uchar_Buffer* b);

FMC_API C_Uchar_Buffer* C_Uchar_Buffer_retain(const C_Uchar_Buffer* b);

FMC_API bool C_Uchar_Buffer_release(C_Uchar_Buffer* *bptr);

FMC_API void C_Uchar_Buffer_set(const C_Uchar_Buffer* *lvalue, const C_Uchar_Buffer* rvalue);

#endif // FMC_UCHARBUF_H_INCLUDED

