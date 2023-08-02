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

#ifndef CCONV_H_INCLUDED
#define CCONV_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <wchar.h>

#define UTF_8   "UTF-8"
#define UTF_32  "UTF-32"

typedef uint16_t utf16_t;

/**
 * Classification scheme for names of character sets.
 * Many encodings have several valid names according to iconv and others;
 * "ASCII" and "US-ASCII" designate the same encoding,
 * as does "UCS-8" and "UTF-8".
 */
typedef enum Charset_Type {
    /** 7-bit ASCII */
    CHARSET_ASCII,
    /* ISO-Latin-1, and all its aliases */
    CHARSET_LATIN_1,
    /* ISO-Latin-2 through -16(?), and all *their* aliases */
    CHARSET_LATIN_2_PLUS,
    /* Other single byte encodings not based on the Latin alphabet */
    CHARSET_SINGLE_BYTE,
    /* Other multi-byte encodings, e.g. Shift-JIS */
    CHARSET_MULTI_BYTE,
    /* UTF-8, UCS-8, and variants */
    CHARSET_UTF_8,
    /* UTF-16, UCS-16, and variants */
    CHARSET_UTF_16,
    /* UTF-16, UCS-16, and variants */
    CHARSET_UTF_16BE,
    /* UTF-16, UCS-16, and variants */
    CHARSET_UTF_16LE,
    /* UTF-32, UCS-32, and variants */
    CHARSET_UTF_32,
    /* UTF-32, UCS-32, and variants */
    CHARSET_UTF_32BE,
    /* UTF-32, UCS-32, and variants */
    CHARSET_UTF_32LE,
    /** Encoding does not have a type assigned. */
    CHARSET_UNKNOWN = 0xFF,
} Charset_Type;

/**
 * Classifies a character set `csname` to facilitate handling.
 * Many character sets have aliases or slight variants with technical
 * differences; others have characteristics that make them less or more
 * likely to be converted to Unicode for ease of processing.
 * Returns the classification.
 */
Charset_Type C_Conv_charset_type(const char* csname);

bool C_Conv_is_ascii(size_t sz, const char* buf);

bool C_Conv_is_utf16(size_t sz, const wchar_t* buf);

size_t C_Conv_utf8_to_16_length(size_t sz, const char* buf, size_t *csz);

size_t C_Conv_utf8_to_32_length(size_t sz, const char* buf, size_t *csz);

size_t C_Conv_utf16_to_8_length(size_t sz, const utf16_t* buf, size_t *csz);

size_t C_Conv_utf32_to_8_length(size_t sz, const wchar_t* buf, size_t *csz);

size_t C_Conv_utf8_to_16(size_t insz, const char* inbuf, size_t outsz, utf16_t* outbuf);

size_t C_Conv_utf8_to_32(size_t insz, const char* inbuf, size_t outsz, wchar_t* outbuf);

size_t C_Conv_utf16_to_8(size_t insz, const utf16_t* inbuf, size_t outsz, char* outbuf);

size_t C_Conv_utf16_to_32(size_t insz, const utf16_t* inbuf, size_t outsz, wchar_t* outbuf);

size_t C_Conv_utf32_to_8(size_t insz, const wchar_t* inbuf, size_t outsz, char* outbuf);

size_t C_Conv_utf32_to_16(size_t insz, const wchar_t* inbuf, size_t outsz, utf16_t* outbuf);

/**
 * Converts `insz` bytes at `inbuf` encoded via character encoding `incode` 
 * to `outbuf` (up to `outsz` bytes) encoded via `outcode`.
 * If given, `nreadp` contains the number of bytes actually read from `inbuf`.
 * The function returns the number of bytes written to `outbuf`.
 */
ssize_t C_Conv_transcode(const char* incode, const char* outcode, size_t insz, char* inbuf, size_t outsz, char* outbuf, size_t* nread);

#endif // CCONV_H_INCLUDED
