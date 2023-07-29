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
#include <wchar.h>
#include "minctest.h"
#include "cconv.h"

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

static const char* wcs2cstr(const wchar_t* wcs) {
    char buf[STRBUFSIZ];
    size_t wlen = wcslen(wcs);
    size_t len = 0;

    bzero(buf, STRBUFSIZ);
    for (int i = 0; i < wlen; i++) {
        len = append_ascii(wcs[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static size_t jcslen(const utf16_t* jcs) {
    int len = 0;
    while (jcs[len] != 0) {
        len++;
    }
    return len;
}

static const char* jcs2cstr(const utf16_t* jcs) {
    char buf[STRBUFSIZ];
    size_t jlen = jcslen(jcs);
    size_t len = 0;

    bzero(buf, STRBUFSIZ);
    for (int i = 0; i < jlen; i++) {
        len = append_ascii(jcs[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static const char* utf2cstr(const char* utf8str) {
    char buf[STRBUFSIZ];
    size_t slen = strlen(utf8str);
    size_t len = 0;

    bzero(buf, STRBUFSIZ);
    for (int i = 0; i < slen; i++) {
        len = append_ascii(0xFF & utf8str[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static const wchar_t* cstr2wcs(const char* s) {
    wchar_t buf[STRBUFSIZ];
    size_t len = strlen(s);
    int i;

    bzero(buf, STRBUFSIZ);
    for (i = 0; i < len; i++) {
        buf[i] = (wchar_t)s[i];
    }
    lequal((int)len, i);
    return (wchar_t*)stralloc(buf, len, sizeof(wchar_t));
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

    lsequal("alpha", wcs2cstr(L"alpha"));
    lsequal("bravo", wcs2cstr(L"bravo"));
    lsequal("charlie", wcs2cstr(L"charlie"));
    lsequal("delta", wcs2cstr(L"delta"));
    lsequal("echo", wcs2cstr(L"echo"));
    lsequal("tsch\\u{fc}\\u{df}", wcs2cstr(L"tschüß"));

    const char* actual = wcs2cstr(cstr2wcs("foxtrot"));
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
    const wchar_t* expect = L"\ufeffa very simple problem";
    size_t nread = 0;
    ssize_t nwrit = 0;
    int errcode;
    size_t outsz = STRBUFSIZ;
    char outbuf[STRBUFSIZ];

    nwrit = C_Conv_transcode(UTF_8, UTF_32, insz, inbuf, outsz, outbuf, &nread);
    errcode = errno;

    lequal(0, errcode);
    lequal((int)insz, (int)nread);
    lequal(92, (int)nwrit);
    lok(wcscmp(expect, (wchar_t*)outbuf) == 0);
    lsequal(wcs2cstr(expect), wcs2cstr((wchar_t*)outbuf));

    free_strings();
}

static void conv_utf8_to_32() {
    // Characters taken from Wikipedia article on UTF-8.
    char* inbuf = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    size_t insz = strlen(inbuf);
    const wchar_t expect[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    ssize_t result = 0;
    int errcode;
    const size_t outsz = STRBUFSIZ/4;
    wchar_t outbuf[outsz];

    bzero(outbuf, sizeof(outbuf));

    result = C_Conv_utf8_to_32(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(13, (int)result);
    lok(wcscmp(expect, (wchar_t*)outbuf) == 0);
    lsequal(wcs2cstr(expect), wcs2cstr((wchar_t*)outbuf));

    free_strings();
}

static void conv_utf32_to_8() {
    // Characters taken from Wikipedia article on UTF-8.
    char* expect = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    const wchar_t inbuf[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    const size_t insz = wcslen(inbuf);
    char outbuf[STRBUFSIZ];
    size_t result;
    int errcode;

    bzero(outbuf, STRBUFSIZ);

    result = C_Conv_utf32_to_8(insz, inbuf, STRBUFSIZ, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(24, (int)result);
    lsequal(expect, outbuf);

    free_strings();
}

static void conv_utf8_to_16() {
    // Characters taken from Wikipedia article on UTF-8.
    char* inbuf = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    size_t insz = strlen(inbuf);
    const utf16_t expect[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const int expectsz = 14;
    ssize_t result = 0;
    int errcode;
    const size_t outsz = STRBUFSIZ/4;
    utf16_t outbuf[outsz];

    bzero(outbuf, sizeof(outbuf));

    result = C_Conv_utf8_to_16(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(expectsz, (int)result);
    lsequal(jcs2cstr(expect), jcs2cstr((utf16_t*)outbuf));

    free_strings();
}

static void conv_utf16_to_8() {
    // Characters taken from Wikipedia article on UTF-8.
    char* expect = 
        "$ \xC2\xA3 \xD0\x98 \xE0\xA4\xB9 \xE2\x82\xAC \xED\x95\x9C \xF0\x90\x8D\x88";
    const utf16_t inbuf[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const size_t insz = jcslen(inbuf);
    char outbuf[STRBUFSIZ];
    size_t result;
    int errcode;

    bzero(outbuf, STRBUFSIZ);

    result = C_Conv_utf16_to_8(insz, inbuf, STRBUFSIZ, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(24, (int)result);
    lsequal(expect, outbuf);
    lsequal(utf2cstr(expect), utf2cstr(outbuf));

    free_strings();
}

static void conv_utf32_to_16() {
    const wchar_t inbuf[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    const int insz = 13;
    const utf16_t expect[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const int expectsz = 14;
    utf16_t outbuf[STRBUFSIZ/2];
    const int outsz = sizeof(outbuf);
    size_t result;
    int errcode;

    bzero(outbuf, outsz);

    result = C_Conv_utf32_to_16(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(expectsz, (int)result);
    lsequal(jcs2cstr(expect), jcs2cstr(outbuf));

    free_strings();
}

static void conv_utf16_to_32() {
    const wchar_t expect[] = { 
        L'$',       L' ', 0x000000A3, L' ', 0x00000418, L' ', 
        0x00000939, L' ', 0x000020AC, L' ', 0x0000D55C, L' ', 
        0x00010348, 0x0,  0x0,        0x0,  0x0,        0x0 };
    const int expectsz = 13;
    const utf16_t inbuf[] = { 
        L'$',   L' ',   0x00A3, L' ', 0x0418, L' ', 
        0x0939, L' ',   0x20AC, L' ', 0xD55C, L' ', 
        0xD800, 0xDF48, 0x0,    0x0,  0x0,    0x0 };
    const int insz = 14;
    wchar_t outbuf[STRBUFSIZ/4];
    const int outsz = sizeof(outbuf);
    size_t result;
    int errcode;

    bzero(outbuf, outsz);

    result = C_Conv_utf16_to_32(insz, inbuf, outsz, outbuf);
    errcode = errno;

    lequal(0, errcode);
    lequal(expectsz, (int)result);
    lsequal(wcs2cstr(expect), wcs2cstr(outbuf));

    free_strings();
}

/**
 * TODO: More error conditions and bad output.
 * - run out of output buffer
 * - malformed UTF-8 input (e.g. too many or not enough continuation bytes)
 * - malformed UTF-16 input (e.g. single surrogate pairs)
 */

int main (int argc, char* argv[]) {
    lrun("cconv_transcode_smoke", conv_smoke);
    lrun("cconv_utf8_to_32", conv_utf8_to_32);
    lrun("cconv_utf32_to_8", conv_utf32_to_8);
    lrun("cconv_utf8_to_16", conv_utf8_to_16);
    lrun("cconv_utf16_to_8", conv_utf16_to_8);
    lrun("cconv_utf32_to_16", conv_utf32_to_16);
    lrun("cconv_utf16_to_32", conv_utf16_to_32);
    lrun("test_code_smoke", string_smoke);
    lresults();
    return lfails != 0;
}
