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

#ifndef FMC_WSTRING_H_INCLUDED
#define FMC_WSTRING_H_INCLUDED

#include "common.h"

/**
 *
 */
typedef struct _U_String U_String;

/**
 * Prototype for an iterator over all chars s[`i`]==`c` in the string `s`.
 * The invoker provides `data`, which is passed ssize_to every invocation
 * until iteration ends.
 * If the function returns false, iteration ends.
 */
typedef bool (*wchar_iterator)(void* data, size_t i, wchar_t c);

/**
 * Create an ASCII string at `sp` with size `sz` characters in `buf`.
 * Returns false if the characters are not ASCII or UTF-8, or the system 
 * runs out of memory; both imply no string can be created.
 */
FMC_API bool U_String_new_ascii(U_String* *sp, size_t sz, const char* buf);

/**
 * Create a UTF-8 string at `sp` with size `sz` characters in `buf`.
 * Returns false if the characters are not ASCII or UTF-8, or the system 
 * runs out of memory; both imply no string can be created.
 */
FMC_API bool U_String_new_utf8(U_String* *sp, size_t sz, const utf8_t* buf);

/**
 * Create a UTF-16 string at `sp` with size `sz` characters in `buf`.
 * Returns false if the system runs out of memory or the string cannot 
 * be created for other reasons.
 */
FMC_API bool U_String_new_utf16(U_String* *sp, size_t sz, const utf16_t* buf);

/**
 * Create a UTF-32 string at `sp` with size `sz` wide characters in `buf`.
 * Returns false if the system runs out of memory or the string cannot 
 * be created for other reasons.
 */
FMC_API bool U_String_new_utf32(U_String* *sp, size_t sz, const wchar_t* buf);

/**
 * Create a string at `sp` with the character encoding `enc` 
 * and size `sz` octets in `buf`.
 * Returns false if the system runs out of memory or the string cannot 
 * be created for other reasons.
 */
FMC_API bool U_String_new_encoded(U_String* *sp, const char* charset, size_t sz, const octet_t* buf);

/**
 * Create a string at `sp` from a null-terminated string `cstr`.
 */
FMC_API bool U_String_new_from_cstring(U_String* *sp, const char* cstr);

/**
 *
 */
FMC_API wchar_t U_String_char_at(U_String* s, size_t i);

/**
 *
 */
FMC_API size_t U_String_length(U_String* s);

/**
 *
 */
FMC_API size_t U_String_to_byte(U_String* s, size_t offset, size_t max, octet_t* buf);

/**
 *
 */
FMC_API size_t U_String_to_utf8(U_String* s, size_t offset, size_t max, utf8_t* buf);

/**
 *
 */
FMC_API size_t U_String_to_utf32(U_String* s, size_t offset, size_t max, utf32_t* buf);

/**
 *
 */
FMC_API ssize_t U_String_to_charset(U_String* s, const char* charset, size_t offset, size_t max, octet_t* buf);

/**
 *
 */
FMC_API size_t U_String_each(U_String* s, void* data, wchar_iterator f);

/**
 *
 */
FMC_API size_t U_String_each_after(U_String* s, size_t index, void* data, wchar_iterator f);

/**
 *
 */
FMC_API bool U_String_slice(U_String* *sp, U_String* s, ssize_t first, ssize_t last);

/**
 *
 */
FMC_API bool U_String_slice_from(U_String* *sp, U_String* s, ssize_t first);

/**
 *
 */
FMC_API bool U_String_slice_to(U_String* *sp, U_String* s, ssize_t last);

/**
 *
 */
FMC_API bool U_String_join(U_String* *sp, U_String* head, U_String* tail);

/**
 *
 */
FMC_API bool U_String_join_n(U_String* *sp, size_t n, ...);

/**
 *
 */
FMC_API bool U_String_is_live(U_String* s);

/**
 *
 */
FMC_API size_t U_String_references(U_String* s);

/**
 *
 */
FMC_API U_String* U_String_retain(U_String* s);

/**
 *
 */
FMC_API bool U_String_release(U_String* *s);

/**
 *
 */
FMC_API void U_String_set(U_String* *lvalue, U_String* rvalue);

#endif // FMC_WSTRING_H_INCLUDED

