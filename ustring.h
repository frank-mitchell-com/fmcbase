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

#ifndef FMC_USTRING_H_INCLUDED
#define FMC_USTRING_H_INCLUDED

#include "common.h"

/*
 * TODO: Adapt for platform endinanness?
 */

#define ASCII   "US-ASCII"
#define UTF_8   "UTF-8"
#define UTF_16  "UTF-16"
#define UTF_32  "UTF-32"

/**
 * A string of Unicode codepoints, i.e. UTF-32 or `char32_t`.
 * The implementation stores strings efficiently if no character in the
 * string needs the whole 4 bytes.
 */
typedef struct _C_Ustring C_Ustring;

/**
 * Create an ASCII string at `sp` with size `sz` characters in `buf`.
 * Returns false if the characters are not ASCII or UTF-8, or the system 
 * runs out of memory; both imply no string can be created.
 */
FMC_API bool C_Ustring_new_ascii(const C_Ustring* *sp, size_t sz, const char* buf);

/**
 * Create a UTF-8 string at `sp` with size `sz` characters in `buf`.
 * Returns false if the characters are not ASCII or UTF-8, or the system 
 * runs out of memory; both imply no string can be created.
 */
FMC_API bool C_Ustring_new_utf8(const C_Ustring* *sp, size_t sz, const char8_t* buf);

/**
 * Create a UTF-16 string at `sp` with size `sz` characters in `buf`.
 * Returns false if the system runs out of memory or the string cannot 
 * be created for other reasons.
 */
FMC_API bool C_Ustring_new_utf16(const C_Ustring* *sp, size_t sz, const char16_t* buf);

/**
 * Create a UTF-32 string at `sp` with size `sz` wide characters in `buf`.
 * Returns false if the system runs out of memory or the string cannot 
 * be created for other reasons.
 */
FMC_API bool C_Ustring_new_utf32(const C_Ustring* *sp, size_t sz, const char32_t* buf);

/**
 * Create a string at `sp` with the character encoding `enc` 
 * and size `sz` octets in `buf`.
 * Returns false if the system runs out of memory or the string cannot 
 * be created for other reasons.
 */
FMC_API bool C_Ustring_new_encoded(const C_Ustring* *sp, const char* charset, size_t sz, const octet_t* buf);

/**
 * Create a string at `sp` from a null-terminated string `cstr`.
 */
FMC_API bool C_Ustring_new_from_cstring(const C_Ustring* *sp, const char* cstr);

/**
 * Returns 0 if `a` == `b`, < 0 if `a` < `b`, and > 0 if `a` > `b`;
 * Longer strings are greater than shorter strings, and higher codepoints
 * are greater than lower codepoints.
 */
FMC_API int C_Ustring_compare(const C_Ustring* a, const C_Ustring* b);

/**
 * Whether `a` and `b` contain the same value.
 */
FMC_API bool C_Ustring_equals(const C_Ustring* a, const C_Ustring* b);

/**
 * A hashcode for `s`.
 */
FMC_API uint64_t C_Ustring_hashcode(const C_Ustring* s);

/**
 * The `i`th character of `s`, starting at 0;
 */
FMC_API char32_t C_Ustring_char_at(const C_Ustring* s, size_t i);

/**
 * The number of characters in `s`.
 */
FMC_API size_t C_Ustring_length(const C_Ustring* s);

/**
 * Convert `s` to a UTF-8 string and write it to `buf`, starting at `offset`
 * and writing only `max` bytes.
 */
FMC_API size_t C_Ustring_to_utf8(const C_Ustring* s, size_t offset, size_t max, char8_t* buf);

/**
 * Convert `s` to a UTF-32 string and write it to `buf`, starting at `offset`
 * and writing only `max` characters.
 */
FMC_API size_t C_Ustring_to_utf32(const C_Ustring* s, size_t offset, size_t max, char32_t* buf);

/**
 * Convert `s` to a string in the named `charset` and write it to `buf`, 
 * starting at `offset`and writing only `max` characters.
 */
FMC_API ssize_t C_Ustring_to_charset(const C_Ustring* s, const char* charset, size_t offset, size_t max, octet_t* buf);

/**
 * Create a substring of `s` from `first` to `last` inclusive, and return it
 * in `*sp`.  If `first` or `last` are negative, the index counts backwards
 * from the end of the string: -1 is the last character, -2 is the second to
 * last character, etc.  If `first` > `last` the resulting string is of
 * length zero.  Likewise if `first` or `last` are off the end of the string,
 * the substring will be truncated at the last character of `s`;
 */
FMC_API bool C_Ustring_slice(const C_Ustring* *sp, const C_Ustring* s, ssize_t first, ssize_t last);

/**
 * Create a substring of `s` from `first` to the end of the string, and return 
 * it in `*sp`.  If `first` is negative, the index counts backwards
 * from the end of the string: -1 is the last character, -2 is the second to
 * last character, etc.  If `first` is off the end of the string,
 * the substring will be zero length;
 */
FMC_API bool C_Ustring_slice_from(const C_Ustring* *sp, const C_Ustring* s, ssize_t first);

/**
 * Create a substring of `s` from the start of the string to `last` and return 
 * it in `*sp`.  If `last` is negative, the index counts backwards
 * from the end of the string: -1 is the last character, -2 is the second to
 * last character, etc.  If `last` is less than 0,
 * the substring will be zero length;
 */
FMC_API bool C_Ustring_slice_to(const C_Ustring* *sp, const C_Ustring* s, ssize_t last);

/**
 * Concatenate `head` and `tail` and return the resulting string in `*sp`;
 */
FMC_API bool C_Ustring_join(const C_Ustring* *sp, const C_Ustring* head, const C_Ustring* tail);

/**
 * Concatenate `n` wstrings and return the resulting string in `*sp`.
 */
FMC_API bool C_Ustring_join_n(const C_Ustring* *sp, size_t n, ...);

/**
 * Whether `s` is still a valid object.
 * False implies the memory location has been freed.
 */
FMC_API bool C_Ustring_is_live(const C_Ustring* s);

/**
 * The reference count of `s`;
 */
FMC_API size_t C_Ustring_references(const C_Ustring* s);

/**
 * Increment the reference count of `s` and return `s`;
 */
FMC_API const C_Ustring* C_Ustring_retain(const C_Ustring* s);

/**
 * Decrement the reference count of `*sp` and set it to NULL;
 */
FMC_API bool C_Ustring_release(const C_Ustring* *sp);

/**
 * Set `*lvalue` with a reference to `rvalue`.
 */
FMC_API void C_Ustring_set(const C_Ustring* *lvalue, const C_Ustring* rvalue);

#endif // FMC_USTRING_H_INCLUDED

