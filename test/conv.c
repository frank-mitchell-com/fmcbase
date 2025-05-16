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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "minctest.h"
#include "convert.h"

#define STRBUFSIZ  512 

typedef struct linkedlist {
    struct linkedlist* tail;
    void*   str;
} list_t;

static list_t* _strhead = NULL;

static void* stralloc(void* buf, size_t len, size_t csiz) {
    void* result = calloc(len+1, csiz);

    memcpy(result, buf, len * csiz);

    list_t* prev = _strhead;
    _strhead = calloc(1, sizeof(list_t));
    _strhead->tail = prev;
    _strhead->str  = result;
    return result;
}

static int append_ascii(unsigned int c, char buf[], int j) {
    int len = j;
    if (c <= 0x7F) {
        buf[j] = (char)c;
        len += 1;
    } else {
        sprintf(&(buf[len]), "\\u{%x}", c); 
        len = strlen(buf);
    }
    return len;
}

static size_t ucslen(const char32_t* ucs) {
    int len = 0;
    while (ucs[len] != 0) {
        len++;
    }
    return len;
}

static const char* ucs2cstr(const char32_t* ucs) {
    char buf[STRBUFSIZ];
    size_t wlen = ucslen(ucs);
    size_t len = 0;

    memset(buf, 0, STRBUFSIZ);
    for (int i = 0; i < wlen; i++) {
        len = append_ascii(ucs[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static size_t jcslen(const char16_t* jcs) {
    int len = 0;
    while (jcs[len] != 0) {
        len++;
    }
    return len;
}

static const char* jcs2cstr(const char16_t* jcs) {
    char buf[STRBUFSIZ];
    size_t jlen = jcslen(jcs);
    size_t len = 0;

    memset(buf, 0, STRBUFSIZ);
    for (int i = 0; i < jlen; i++) {
        len = append_ascii(jcs[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static const char* utf2cstr(const char* utf8str) {
    char buf[STRBUFSIZ];
    size_t slen = strlen(utf8str);
    size_t len = 0;

    memset(buf, 0, STRBUFSIZ);
    for (int i = 0; i < slen; i++) {
        len = append_ascii(0xFF & utf8str[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static const char32_t* cstr2ucs(const char* s) {
    char32_t buf[STRBUFSIZ];
    size_t len = strlen(s);
    int i;

    memset(buf, 0, STRBUFSIZ);
    for (i = 0; i < len; i++) {
        buf[i] = (char32_t)s[i];
    }
    lequal((int)len, i);
    return (char32_t*)stralloc(buf, len, sizeof(char32_t));
}

static const char8_t* ucs2utf8(const char32_t* s) {
    size_t bsz;
    char8_t buf[STRBUFSIZ];

    memset(buf, 0, sizeof(buf));
    bsz = C_Conv_char32_to_8(ucslen(s), s, STRBUFSIZ, buf);

    return (char8_t*)stralloc(buf, bsz, sizeof(char8_t));
}

static const char16_t* ucs2utf16(const char32_t* s) {
    size_t bsz;
    char16_t buf[STRBUFSIZ/2];

    memset(buf, 0, sizeof(buf));
    bsz = C_Conv_char32_to_16(ucslen(s), s, STRBUFSIZ, buf);

    return (char16_t*)stralloc(buf, bsz, sizeof(char16_t));
}

static int free_strings() {
    int count = 0;
    list_t* head = _strhead;

    _strhead = NULL;

    while (head != NULL) {
        list_t* prev = head;
        head = head->tail;
        free(prev->str);
        free(prev);
        count++;
    }
    return count;
}

static void string_smoke() {
    int count = 0;

    lsequal("alpha", ucs2cstr(U"alpha"));
    lsequal("bravo", ucs2cstr(U"bravo"));
    lsequal("charlie", ucs2cstr(U"charlie"));
    lsequal("delta", ucs2cstr(U"delta"));
    lsequal("echo", ucs2cstr(U"echo"));
    lsequal("tsch\\u{fc}\\u{df}", ucs2cstr(U"tschüß"));

    const char* actual = ucs2cstr(cstr2ucs("foxtrot"));
    lsequal("foxtrot", actual);

    for (list_t* head = _strhead; head != NULL; head = head->tail) {
        count++;
    }
    lequal(8, count);

    lequal(count, free_strings());
    lok(_strhead == NULL);
}

static void conv_smoke() {
    char* inbuf = "a very simple problem";
    size_t insz = strlen(inbuf) + 1;
    const char32_t* expect = U"\ufeffa very simple problem";
    ssize_t nread = 0;
    ssize_t nwrit = 0;
    int errcode;
    size_t outsz = STRBUFSIZ;
    char outbuf[STRBUFSIZ];

    nwrit = C_Conv_transcode(UTF_8, UTF_32, insz, inbuf, outsz, outbuf, &nread);
    errcode = errno;

    lequal(0, errcode);
    lequal((int)insz, (int)nread);
    lequal(92, (int)nwrit);
    lok(memcmp(expect, outbuf, ucslen(expect)) == 0);
    lsequal(ucs2cstr(expect), ucs2cstr((char32_t*)outbuf));

    free_strings();
}

static void conv_char8_to_32() {
    // Characters taken from Wikipedia article on UTF-8.
    char* inbuf = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    size_t insz = strlen(inbuf);
    const char32_t expect[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    ssize_t result = 0;
    int errcode;
    const size_t outsz = STRBUFSIZ/4;
    char32_t outbuf[outsz];

    memset(outbuf, 0, sizeof(outbuf));

    result = C_Conv_char8_to_32(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(13, (int)result);
    lok(memcmp(expect, outbuf, ucslen(expect)) == 0);
    lsequal(ucs2cstr(expect), ucs2cstr((char32_t*)outbuf));

    free_strings();
}

static void conv_char32_to_8() {
    // Characters taken from Wikipedia article on UTF-8.
    char* expect = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    const char32_t inbuf[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    const size_t insz = ucslen(inbuf);
    char outbuf[STRBUFSIZ];
    size_t result;
    int errcode;

    memset(outbuf, 0, STRBUFSIZ);

    result = C_Conv_char32_to_8(insz, inbuf, STRBUFSIZ, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(24, (int)result);
    lsequal(expect, outbuf);

    free_strings();
}

static void conv_char8_to_16() {
    // Characters taken from Wikipedia article on UTF-8.
    char* inbuf = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    size_t insz = strlen(inbuf);
    const char16_t expect[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const int expectsz = 14;
    ssize_t result = 0;
    int errcode;
    const size_t outsz = STRBUFSIZ/4;
    char16_t outbuf[outsz];

    memset(outbuf, 0, sizeof(outbuf));

    result = C_Conv_char8_to_16(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(expectsz, (int)result);
    lsequal(jcs2cstr(expect), jcs2cstr((char16_t*)outbuf));

    free_strings();
}

static void conv_char16_to_8() {
    // Characters taken from Wikipedia article on UTF-8.
    char* expect = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    const char16_t inbuf[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const size_t insz = jcslen(inbuf);
    char outbuf[STRBUFSIZ];
    size_t result;
    int errcode;

    memset(outbuf, 0, STRBUFSIZ);

    result = C_Conv_char16_to_8(insz, inbuf, STRBUFSIZ, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(24, (int)result);
    lsequal(expect, outbuf);
    lsequal(utf2cstr(expect), utf2cstr(outbuf));

    free_strings();
}

static void conv_char32_to_16() {
    const char32_t inbuf[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    const int insz = 13;
    const char16_t expect[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const int expectsz = 14;
    char16_t outbuf[STRBUFSIZ/2];
    const int outsz = sizeof(outbuf);
    size_t result;
    int errcode;

    memset(outbuf, 0, outsz);

    result = C_Conv_char32_to_16(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(expectsz, (int)result);
    lsequal(jcs2cstr(expect), jcs2cstr(outbuf));

    free_strings();
}

static void conv_char16_to_32() {
    const char32_t expect[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    const int expectsz = 13;
    const char16_t inbuf[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const int insz = 14;
    char32_t outbuf[STRBUFSIZ/4];
    const int outsz = sizeof(outbuf);
    size_t result;
    int errcode;

    memset(outbuf, 0, outsz);

    result = C_Conv_char16_to_32(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(expectsz, (int)result);
    lsequal(ucs2cstr(expect), ucs2cstr(outbuf));

    free_strings();
}

/*
 * TODO: More error conditions and bad output.
 * - run out of output buffer
 * - malformed UTF-8 input (e.g. too many or not enough continuation bytes)
 * - malformed UTF-16 input (e.g. single surrogate pairs)
 */

static void conv_is_ascii() {
    const char *test1 = "This is ASCII";
    const char *test2 = "This (\xC2\xA3) is not ASCII";

    lequal(true, C_Conv_is_ascii(strlen(test1), test1));
    lequal(false, C_Conv_is_ascii(strlen(test2), test2));
}

static char32_t PLANE_1_STRING[] = { 
    'P', 'l', 'a', 'n', 'e', ' ', '1', ':', ' ', 0x10348, '!', 0x0
};

static void conv_length_8_to_32() {
    const char32_t* expect[] = {
        PLANE_1_STRING,
        U"This has Unicode: \u0024\u20AC ...",
        U"This does not."
    };
    const int expectsz = sizeof(expect) / sizeof (const char32_t*);

    for (int i = 0; i < expectsz; i++) {
        const char8_t*  istr = ucs2utf8(expect[i]);
        int            ilen = strlen(istr);
        const char32_t* estr = expect[i];
        int            elen = ucslen(estr);
        size_t csz;

        lequal((int)elen, (int)C_Conv_char8_to_32_length(ilen, istr, &csz));
        lequal((int)ilen, (int)csz);
    }

    free_strings();
}

static void conv_length_8_to_16() {
    const char32_t* expect[] = {
        PLANE_1_STRING,
        U"This has Unicode: \u0024\u20AC ...",
        U"This does not."
    };
    const int expectsz = sizeof(expect) / sizeof (const char32_t*);

    for (int i = 0; i < expectsz; i++) {
        const char8_t*  istr = ucs2utf8(expect[i]);
        int            ilen = strlen(istr);
        const char16_t* estr = ucs2utf16(expect[i]);
        int            elen = jcslen(estr);
        size_t csz;

        lequal((int)elen, (int)C_Conv_char8_to_16_length(ilen, istr, &csz));
        lequal((int)ilen, (int)csz);
    }

    free_strings();
}

static void conv_length_16_to_8() {
    const char32_t* expect[] = {
        PLANE_1_STRING,
        U"This has Unicode: \u0024\u20AC ...",  // 24 char32_t
        U"This does not."                       // 14 char32_t
    };
    const int expectsz = sizeof(expect) / sizeof (const char32_t*);

    for (int i = 0; i < expectsz; i++) {
        const char16_t* istr = ucs2utf16(expect[i]);
        int            ilen = jcslen(istr);
        const char8_t*  estr = ucs2utf8(expect[i]);
        int            elen = strlen(estr);
        size_t csz;

        lequal((int)elen, (int)C_Conv_char16_to_8_length(ilen, istr, &csz));
        lequal((int)ilen, (int)csz);
    }

    free_strings();
}

static void conv_length_32_to_8() {
    const char32_t* input[] = {
        PLANE_1_STRING,
        U"This has Unicode: \u0024\u20AC ...",   // 24 char32_t
        U"This does not."                        // 14 char32_t
    };
    const int inputsz = sizeof(input) / sizeof (const char32_t*);

    for (int i = 0; i < inputsz; i++) {
        const char32_t* istr = input[i];
        int            ilen = ucslen(istr);
        const char8_t*  estr = ucs2utf8(input[i]);
        int            elen = strlen(estr);
        size_t csz;

        lequal((int)elen, (int)C_Conv_char32_to_8_length(ilen, istr, &csz));
        lequal((int)ilen, (int)csz);
    }

    free_strings();
}

static void conv_min_bytes() {
    struct min_bytes_pair {
        int     expect;
        char32_t *data;
    } test[] = {
        { 4, PLANE_1_STRING },
        { 2, U"Unicode: \u0024\u20AC ..." },
        { 2, U"Unicode trap: \u0100 ..." },
        { 1, U"Latin 1: tschüß! \u00FF" },
        { 1, U"ASCII: blah blahbidy blah" },
        { 0, U"" }
    };
    const int testsz = sizeof(test) / sizeof (struct min_bytes_pair);

    for (int i = 1; i < testsz; i++) {
        const char32_t* t32 = test[i].data;
        const char16_t* t16 = ucs2utf16(t32);
        const char8_t*  t8  = ucs2utf8(t32);

        lequal(test[i].expect, C_Conv_min_bytes(ucslen(t32), t32));
        lequal(test[i].expect, C_Conv_min_bytes_utf16(jcslen(t16), t16));
        lequal(test[i].expect, C_Conv_min_bytes_utf8(strlen(t8), t8));
    }

    free_strings();
}


int main (int argc, char* argv[]) {
    lrun("cconv_transcode_smoke", conv_smoke);
    lrun("cconv_char8_to_32", conv_char8_to_32);
    lrun("cconv_char32_to_8", conv_char32_to_8);
    lrun("cconv_char8_to_16", conv_char8_to_16);
    lrun("cconv_char16_to_8", conv_char16_to_8);
    lrun("cconv_char32_to_16", conv_char32_to_16);
    lrun("cconv_char16_to_32", conv_char16_to_32);
    lrun("cconv_is_ascii", conv_is_ascii);
    lrun("cconv_length_8_to_32", conv_length_8_to_32);
    lrun("cconv_length_8_to_16", conv_length_8_to_16);
    lrun("cconv_length_16_to_8", conv_length_16_to_8);
    lrun("cconv_length_32_to_8", conv_length_32_to_8);
    lrun("cconv_min_bytes", conv_min_bytes);
    lrun("test_code_smoke", string_smoke);
    lresults();
    return lfails != 0;
}
