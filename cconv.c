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

//TODO: Conditionally include ICONV, with alternatives.

#include <iconv.h>
#include <string.h>
#include "cconv.h"

extern bool C_Conv_is_ascii(size_t sz, uint8_t* buf) {
    for (int i = 0; i < sz; i++) {
        if (buf[i] > 127) {
            return false;
        }
    }
    return true;
}

extern size_t C_Conv_utf8_to_16_length(size_t sz, uint8_t* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        uint8_t c = buf[i];
        // TODO: Doesn't check if each lead byte has the right number of
        // trailing bytes, especially at the end.
        if (c <= 127 || (c >= 0xC0 && c <= 0xEF)) {
            // Start of a code point beween U+0000 and U+FFFF
            result++;
        } else if (c > 0xEF) {
            // Start of a code point beyond U+FFFF
            result += 2;
        }
    }
    if (csz) *csz = i;
    return result;
}

extern size_t C_Conv_utf8_to_32_length(size_t sz, uint8_t* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        uint8_t c = buf[i];
        // TODO: Doesn't check if each lead byte has the right number of
        // trailing bytes, especially at the end.
        if (c <= 127 || c >= 0xC0) {
            // Start of a code point beween U+0000 and U+FFFF
            result++;
        }
    }
    if (csz) *csz = i;
    return result;
}

extern size_t C_Conv_utf16_to_8_length(size_t sz, uint16_t* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        uint32_t c = buf[i];
        if (c <= 0x7f) {
            result += 1;
        } else if (c <= 0x7ff) {
            result += 2;
        } else {
            result += 3;
        }
    }
    if (csz) *csz = i;
    return result;
}

extern size_t C_Conv_utf32_to_8_length(size_t sz, uint32_t* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        uint32_t c = buf[i];
        if (c <= 0x7f) {
            result += 1;
        } else if (c <= 0x7ff) {
            result += 2;
        } else if (c <= 0xffff) {
            result += 3;
        } else if (c <= 0x10ffff) {
            result += 4;
        } else {
            // Not defined by the Unicode Standard ... yet.
            result += 5;
        }
    }
    if (csz) *csz = i;
    return result;
}

extern ssize_t C_Conv_transcode(const char* incode, const char* outcode, size_t insz, uint8_t* inbuf, size_t outsz, uint8_t* outbuf) {
    iconv_t cd;
    char tocode[101];
    char* inbufp;
    char* outbufp;
    size_t inszp;
    size_t outszp;
    size_t result = 0;

    // Tell `iconv` to transliterate characters not in `outcode`
    // to their closest equivalents.
    strncpy(tocode, incode, 100);
    strncat(tocode, "//TRANSLIT", 100 - strlen(incode));

    inbufp = (char *) inbuf;
    outbufp = (char *) outbuf;
    inszp = insz;
    outszp = outsz;

    cd = iconv_open(tocode, incode);

    result = iconv(cd, &inbufp, &inszp, &outbufp, &outszp);

    iconv_close(cd);

    return result;
}

