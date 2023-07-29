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

#ifndef USTRING_H_INCLUDED
#define USTRING_H_INCLUDED

#include <stdbool.h> /* defines bool */
#include <stddef.h>  /* defines int */
#include <stdint.h>  /* defines uint8_t, utf16_t, uintptr_t */
#include <wchar.h>   /* wchar_t */

#include "csymbol.h"

#define USTR_API    extern

typedef uint8_t  octet_t;
typedef uint8_t  utf8_t;
typedef uint16_t utf16_t;

typedef uintptr_t ustring_t;

typedef struct U_Scope* uscope_t;

#define U_STRING_NULL   ((ustring_t)0)


USTR_API ustring_t U_String_new_ascii(size_t sz, const char* buf);

USTR_API ustring_t U_String_new_utf8(size_t sz, const utf8_t* buf);

USTR_API ustring_t U_String_new_utf16(size_t sz, const utf16_t* buf);

USTR_API ustring_t U_String_new_utf32(size_t sz, const wchar_t* buf);

USTR_API ustring_t U_String_new_encoded(C_Symbol* encoding, size_t sz, const octet_t* buf);

/* Create from a null-terminated string */
USTR_API ustring_t U_String_new_from_cstring(const char* cstr);

USTR_API wchar_t U_String_char_at(ustring_t s, size_t i);

USTR_API C_Symbol* U_String_encoding(ustring_t s);

USTR_API const char* U_String_encoding_name(ustring_t s);

USTR_API size_t U_String_length(ustring_t s);

USTR_API size_t U_String_to_byte(ustring_t s, size_t offset, size_t max, octet_t* buf);
USTR_API size_t U_String_to_utf8(ustring_t s, size_t offset, size_t max, utf8_t* buf);
USTR_API size_t U_String_to_utf32(ustring_t s, size_t offset, size_t max, wchar_t* buf);

USTR_API void U_String_each(ustring_t s, void* data, void (*iter)(void*, size_t, wchar_t));

USTR_API ustring_t U_String_slice(ustring_t s, int first, int last);
USTR_API ustring_t U_String_slice_from(ustring_t s, int first);
USTR_API ustring_t U_String_slice_to(ustring_t s, int last);

USTR_API ustring_t U_String_join(ustring_t head, ustring_t tail);
USTR_API ustring_t U_String_join_n(size_t n, ...);

USTR_API bool U_String_is_live(ustring_t s);
USTR_API size_t U_String_references(ustring_t s);

USTR_API ustring_t U_String_retain(ustring_t s);
USTR_API ustring_t U_String_release(ustring_t s);
USTR_API ustring_t U_String_set(ustring_t *lvalue, ustring_t rvalue);

USTR_API void U_Global_Scope_clean();

USTR_API void U_Scope_begin(uscope_t *sptr);
USTR_API void U_Scope_end(uscope_t *sptr);

#endif // USTRING_H_INCLUDED

