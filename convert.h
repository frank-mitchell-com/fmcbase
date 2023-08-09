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

#ifndef FMC_CONVERT_INCLUDED
#define FMC_CONVERT_INCLUDED

#include "common.h"

/**
 * Determine whether a string is pure ASCII.
 * `false` implies UTF-8 or a Latin encoding.
 */
FMC_API bool C_Conv_is_ascii(size_t sz, const char* buf);

/**
 * Determine the minimum number of bytes required to represent all `sz`
 * characters in UTF-16 string `buf`.
 */
FMC_API unsigned int C_Conv_min_bytes(size_t sz, const utf32_t* buf);

/**
 * Determine the minimum number of bytes required to represent all `sz`
 * characters in UTF-16 string `buf`.
 */
FMC_API unsigned int C_Conv_min_bytes_utf16(size_t sz, const utf16_t* buf);

/**
 * Determine the minimum number of bytes required to represent all `sz`
 * characters in UTF-8 string `buf`.
 */
FMC_API unsigned int C_Conv_min_bytes_utf8(size_t sz, const utf8_t* buf);

/**
 * The length of the resulting string if the first `sz` UTF-8 characters
 * in `buf` were converted to 16-bit characters.
 */
FMC_API size_t C_Conv_utf8_to_16_length(size_t sz, const utf8_t* buf, size_t *csz);

/**
 * The length of the resulting string if the first `sz` UTF-8 characters
 * in `buf` were converted to UTF-32.
 */
FMC_API size_t C_Conv_utf8_to_32_length(size_t sz, const utf8_t* buf, size_t *csz);

/**
 * The length of the resulting string if the first `sz` UTF-16 characters
 * in `buf` were converted to UTF-8.
 */
FMC_API size_t C_Conv_utf16_to_8_length(size_t sz, const utf16_t* buf, size_t *csz);

/**
 * The length of the resulting string if the first `sz` UTF-32 characters
 * in `buf` were converted to UTF-8.
 */
FMC_API size_t C_Conv_utf32_to_8_length(size_t sz, const utf32_t* buf, size_t *csz);


FMC_API size_t C_Conv_utf8_to_16(size_t insz, const utf8_t* inbuf, size_t outsz, utf16_t* outbuf);

FMC_API size_t C_Conv_utf8_to_32(size_t insz, const utf8_t* inbuf, size_t outsz, utf32_t* outbuf);

FMC_API size_t C_Conv_utf16_to_8(size_t insz, const utf16_t* inbuf, size_t outsz, utf8_t* outbuf);

FMC_API size_t C_Conv_utf16_to_32(size_t insz, const utf16_t* inbuf, size_t outsz, utf32_t* outbuf);

FMC_API size_t C_Conv_utf32_to_8(size_t insz, const utf32_t* inbuf, size_t outsz, utf8_t* outbuf);

FMC_API size_t C_Conv_utf32_to_16(size_t insz, const utf32_t* inbuf, size_t outsz, utf16_t* outbuf);

/**
 * Converts `insz` bytes at `inbuf` encoded via character encoding `incode` 
 * to `outbuf` (up to `outsz` bytes) encoded via `outcode`.
 * If given, `nreadp` contains the number of bytes actually read from `inbuf`.
 * The function returns the number of bytes written to `outbuf`.
 */
FMC_API ssize_t C_Conv_transcode(const char* incode, const char* outcode, size_t insz, octet_t* inbuf, size_t outsz, octet_t* outbuf, ssize_t* nread);

#endif // FMC_CONVERT_INCLUDED
