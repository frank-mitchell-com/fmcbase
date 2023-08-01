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

#include <errno.h>
#include <iconv.h>
#include <string.h>
#include "cconv.h"

#define OCTET(x) ((uint8_t)(0xff & (x)))

/* ---------------------- Buffer Size Helpers --------------------------- */

extern bool C_Conv_is_ascii(size_t sz, const char* buf) {
    for (int i = 0; i < sz; i++) {
        if (buf[i] > 127) {
            return false;
        }
    }
    return true;
}

extern bool C_Conv_is_utf16(size_t sz, const wchar_t* buf) {
    for (int i = 0; i < sz; i++) {
        if (buf[i] > 0xFFFF) {
            return false;
        }
    }
    return true;
}

extern size_t C_Conv_utf8_to_16_length(size_t sz, const char* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        uint32_t c = OCTET(buf[i]);
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

extern size_t C_Conv_utf8_to_32_length(size_t sz, const char* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        uint32_t c = OCTET(buf[i]);
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

extern size_t C_Conv_utf16_to_8_length(size_t sz, const utf16_t* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        wchar_t c = buf[i];
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

extern size_t C_Conv_utf32_to_8_length(size_t sz, const wchar_t* buf, size_t *csz) {
    int i, result = 0;
    for (i = 0; i < sz; i++) {
        wchar_t c = buf[i];
        if (c <= 0x7f) {
            result += 1;
        } else if (c <= 0x7ff) {
            result += 2;
        } else if (c <= 0xffff) {
            result += 3;
        } else {
            // Higher planes not defined by the Unicode Standard ... yet.
            result += 4;
        }
    }
    if (csz) *csz = i;
    return result;
}

/* -------------------------- UTF-x Conversions -------------------------- */

static bool has_conbytes(const char* buf, int i, size_t count, size_t max) {
    for (int k = 1; k <= count; k++) {
        if (i+k >= max) {
            return false;
        }
        uint8_t c = OCTET(buf[i+k]);
        if (c < 0x80 || c >= 0xC0) {
            return false;
        }
    }
    return true;
}

static int read_utf8(wchar_t* cpp, size_t insz, const char* inbuf, size_t i) {
    uint8_t c = OCTET(inbuf[i]);
    if (c <= 0x7F) {
        (*cpp) = c;
        return 1;
    } else if (c >= 0xC0 && c < 0xE0 && has_conbytes(inbuf, i, 1, insz)) {
        uint32_t c2 = OCTET(inbuf[i+1]);
        (*cpp) = (wchar_t)(((c & 0x1F) << 6) | (c2 & 0x3F));
        return 2;
    } else if (c >= 0xE0 && c < 0xF0 && has_conbytes(inbuf, i, 2, insz)) {
        uint32_t c2 = OCTET(inbuf[i+1]);
        uint32_t c3 = OCTET(inbuf[i+2]);
        (*cpp) = 
            (wchar_t)(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F));
        return 3;
    } else if (c >= 0xF0 && c < 0xF8 && has_conbytes(inbuf, i, 3, insz)) {
        uint32_t c2 = OCTET(inbuf[i+1]);
        uint32_t c3 = OCTET(inbuf[i+2]);
        uint32_t c4 = OCTET(inbuf[i+3]);
        (*cpp) = (wchar_t)(((c & 0x07) << 18) 
                        | ((c2 & 0x3F) << 12) 
                        | ((c3 & 0x3F) << 6) 
                        | (c4 & 0x3F));
        return 4;
    } else if (c >= 0xF8 && c < 0xFC && has_conbytes(inbuf, i, 4, insz)) {
        uint32_t c2 = OCTET(inbuf[i+1]);
        uint32_t c3 = OCTET(inbuf[i+2]);
        uint32_t c4 = OCTET(inbuf[i+3]);
        uint32_t c5 = OCTET(inbuf[i+4]);
        (*cpp) = (wchar_t)(((c & 0x03) << 24) 
                        | ((c2 & 0x3F) << 18) 
                        | ((c3 & 0x3F) << 12) 
                        | ((c4 & 0x3F) << 6)
                        | (c5 & 0x3F));
        return 5;
    } else if (c >= 0xFC && c < 0xFE && has_conbytes(inbuf, i, 5, insz)) {
        uint32_t c2 = OCTET(inbuf[i+1]);
        uint32_t c3 = OCTET(inbuf[i+2]);
        uint32_t c4 = OCTET(inbuf[i+3]);
        uint32_t c5 = OCTET(inbuf[i+4]);
        uint32_t c6 = OCTET(inbuf[i+5]);
        (*cpp) = (wchar_t)(((c & 0x03) << 30) 
                        | ((c2 & 0x3F) << 24) 
                        | ((c3 & 0x3F) << 28) 
                        | ((c4 & 0x3F) << 12)
                        | ((c5 & 0x3F) << 6)
                        | (c6 & 0x3F));
        return 6;
    }
    // TODO: Set errno
    return 0;
}

static int write_utf8(wchar_t cp, size_t outsz, char* outbuf, size_t j) {
    if (cp <= 0x7f) {
        outbuf[j] = cp;
        return 1;
    } else if (cp <= 0x7ff && j+2 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xC0 | (0x1F & (cp >> 6)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & cp));
        return 2;
    } else if (cp <= 0xffff && j+3 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xE0 | (0x0F & (cp >> 12)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & cp));
        return 3;
    } else if (cp <= 0x1fffff && j+4 < outsz) {
        // Anything above 0x10ffff not defined by the Unicode Standard 
        // ... yet.
        outbuf[j + 0] = (uint8_t) (0xF0 | (0x07 & (cp >> 18)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 12)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 3] = (uint8_t) (0x80 | (0x3F & cp));
        return 4;
    } else if (cp <= 0x3fffff && j+5 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xF8 | (0x03 & (cp >> 24)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 18)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & (cp >> 12)));
        outbuf[j + 3] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 4] = (uint8_t) (0x80 | (0x3F & cp));
        return 5;
    } else if (cp <= 0x7fffff && j+6 < outsz) {
        outbuf[j + 0] = (uint8_t) (0xFC | (0x01 & (cp >> 24)));
        outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & (cp >> 18)));
        outbuf[j + 2] = (uint8_t) (0x80 | (0x3F & (cp >> 12)));
        outbuf[j + 3] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 4] = (uint8_t) (0x80 | (0x3F & (cp >> 6)));
        outbuf[j + 5] = (uint8_t) (0x80 | (0x3F & cp));
        return 6;
    }
    // TODO: Set errno?
    return 0;
}

extern size_t C_Conv_utf8_to_32(size_t insz, const char* inbuf, size_t outsz, wchar_t* outbuf) {
    size_t i = 0;
    size_t j;
    for (j = 0; i < insz && j < outsz; j++) {
        wchar_t cp = 0;
        int inci = read_utf8(&cp, insz, inbuf, i);

        if (inci <= 0) {
            break;
        }
        i += inci;

        outbuf[j] = cp;
    }
    return j;
}

static bool is_high_surrogate(utf16_t v) {
    return v >= 0xD800 && v <= 0xDBFF;
}

static bool is_low_surrogate(utf16_t v) {
    return v >= 0xDC00 && v <= 0xDFFF;
}

static bool is_surrogate(utf16_t v) {
    return v >= 0xD800 && v <= 0xDFFF;
}

static utf16_t high_surrogate(wchar_t v) {
    return ((v - 0x10000) >> 10) + 0xD800;
}

static utf16_t low_surrogate(wchar_t v) {
    return (v - 0x10000) + 0xDC00;
}

static wchar_t surrogate_pair(utf16_t high, utf16_t low) {
    return (wchar_t)0x10000 
                + (((high - 0xD800) << 10)) 
                + ((low - 0xDC00));
}

static int read_utf16(wchar_t *cpp, size_t insz, const utf16_t* inbuf, size_t i) {
    wchar_t cp = inbuf[i];
    if (!is_surrogate(cp)) {
        (*cpp) = cp;
        return 1;
    } else {
        wchar_t cp2 = inbuf[i+1];
        if (is_high_surrogate(cp) && is_low_surrogate(cp2)) {
            (*cpp) = surrogate_pair(cp, cp2);
            return 2;
        } else if (is_high_surrogate(cp2) && is_low_surrogate(cp)) {
            (*cpp) = surrogate_pair(cp2, cp);
            return 2;
        } else {
            // TODO: Set errno?
            return 0;
        }
    }
}

static int write_utf16(wchar_t cp, size_t outsz, utf16_t* outbuf, size_t j) {
    if (cp <= 0xFFFF) {
        outbuf[j] = cp;
        return 1;
    } else if (j+1 < outsz) {
        outbuf[j]   = high_surrogate(cp);
        outbuf[j+1] = low_surrogate(cp);
        return 2;
    } else {
        // TODO: Set errno?
        return 0;
    }
}

extern size_t C_Conv_utf8_to_16(size_t insz, const char* inbuf, size_t outsz, utf16_t* outbuf) {
    int i = 0;
    int j = 0;
    while (i < insz && j < outsz) {
        wchar_t cp = 0;
        int inci, incj;

        inci = read_utf8(&cp, insz, inbuf, i);
        if (inci <= 0) {
            break;
        }
        i += inci;

        incj = write_utf16(cp, outsz, outbuf, j);
        if (incj <= 0) {
            break;
        }
        j += incj;
    }
    return j;
}

extern size_t C_Conv_utf16_to_8(size_t insz, const utf16_t* inbuf, size_t outsz, char* outbuf) {
    int i = 0;
    int j = 0;
    while (i < insz && j < outsz) {
        wchar_t cp;
        int inci, incj;

        inci = read_utf16(&cp, insz, inbuf, i);
        if (inci <= 0) {
            break;
        }
        i += inci;

        incj = write_utf8(cp, outsz, outbuf, j);
        if (incj <= 0) {
            break;
        }
        j += incj;
    }
    return j;
}

extern size_t C_Conv_utf32_to_16(size_t insz, const wchar_t* inbuf, size_t outsz, utf16_t* outbuf) {
    int j = 0;
    for (int i = 0; i < insz && j < outsz; i++) {
        int incj = write_utf16(inbuf[i], outsz, outbuf, j);

        if (incj <= 0) {
            break;
        }
        j += incj;
    }
    return j;
}

extern size_t C_Conv_utf16_to_32(size_t insz, const utf16_t* inbuf, size_t outsz, wchar_t* outbuf) {
    int i = 0;
    int j = 0;
    for (j = 0; i < insz && j < outsz; j++) {
        wchar_t cp;
        int inci = read_utf16(&cp, insz, inbuf, i);

        if (inci <= 0) {
            break;
        }
        i += inci;

        outbuf[j] = cp;
    }
    return j;
}

extern size_t C_Conv_utf32_to_8(size_t insz, const wchar_t* inbuf, size_t outsz, char* outbuf) {
    size_t i;
    size_t j = 0;
    for (i = 0; i < insz && j < outsz; i++) {
        int incj = write_utf8(inbuf[i], outsz, outbuf, j);

        if (incj <= 0) {
            break;
        }
        j += incj;
    }
    return j;
}

/* ----------------------- GENERAL CONVERSION ----------------------------*/

extern ssize_t C_Conv_transcode(const char* incode, const char* outcode, size_t insz, char* inbuf, size_t outsz, char* outbuf, size_t* nreadp) {
    iconv_t cd;
    char tocode[101];
    char* inbufp = inbuf;
    char* outbufp = outbuf;
    size_t inszp = insz;
    size_t outszp = outsz;

    // Tell `iconv` to transliterate characters not in `outcode`
    // to their closest equivalents.
    strncpy(tocode, outcode, 100);
    strncat(tocode, "//TRANSLIT", 100 - strlen(incode));

    errno = 0;

    cd = iconv_open(tocode, incode);

    if (cd < 0 || errno != 0) {
        return -1;
    }

    iconv(cd, &inbufp, &inszp, &outbufp, &outszp);

    iconv_close(cd);

    if (nreadp) *nreadp = inbufp - inbuf;

    return outbufp - outbuf;
}

