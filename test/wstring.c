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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>
#include "minctest.h"
#include "convert.h"
#include "wstring.h"

#define STRBUFSIZ   64

/* ------------------------ TEST DATA ----------------------------- */

typedef struct string_data {
    const size_t   len;
    const char*    input;
    const char*    charset;
    const wchar_t* expect;
} string_data;

string_data EXPECT[] = {
    {  0, "",                   "LATIN1",       L""},
    {  5, "alpha",              "US-ASCII",     L"alpha"},
    {  4, "beta",               "US-ASCII",     L"beta"},
    {  5, "gamma",              "ASCII",        L"gamma"},
    {  5, "delta",              "US-ASCII",     L"delta"},
    { 14, "verisimilitude",     "US-ASCII",     L"verisimilitude"},
    {  6, "tsch\xfc\xdf",       "LATIN1",       L"tschüß"},
    {  4, "\x20\xAC\x00?",      "UCS-2BE",      L"\u20AC?"},
    {  4, "\xAC\x20?\x00",      "UCS-2LE",      L"\u20AC?"},
    {  4, "\x00\x00\xD5\x5C",   "UCS-4BE",      L"\uD55C"},
};

const int EXPECTSZ = sizeof(EXPECT)/sizeof(string_data);

/* --------------------------- SUPPORT CODE ------------------------- */

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

static const utf8_t* wcs2utf8(const wchar_t* wcs) {
    utf8_t buf[STRBUFSIZ+2];
    size_t wlen = wcslen(wcs);
    size_t len;

    bzero(buf, sizeof(buf));
    len = C_Conv_utf32_to_8(wlen, wcs, STRBUFSIZ, buf);
    return (utf8_t*)stralloc(buf, len, sizeof(char));
}

static const utf16_t* wcs2utf16(const wchar_t* wcs) {
    utf16_t buf[STRBUFSIZ+2];
    size_t wlen = wcslen(wcs);
    size_t len;

    bzero(buf, sizeof(buf));
    len = C_Conv_utf32_to_16(wlen, wcs, STRBUFSIZ, buf);
    return (utf16_t*)stralloc(buf, len, sizeof(utf16_t));
}

static const size_t jcslen(const utf16_t* jcs) {
    int i = 0;
    while (jcs[i] != 0) {
        i++;
    }
    return i;
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

/* ------------------------------ TESTS ----------------------------- */

static void string_smoke() {
    const C_Wstring* s = NULL;
    uint8_t outbuf[STRBUFSIZ];

    lok(C_Wstring_new_from_cstring(&s, "alpha"));
    lok(s != NULL);

    lok(C_Wstring_release(&s));
    lok(s == NULL);
}

static void string_chars() {
    const C_Wstring* s = NULL;
    const wchar_t* expect = L"alpha";
    int expectsz = wcslen(expect);
    uint8_t outbuf[STRBUFSIZ];

    lok(C_Wstring_new_from_cstring(&s, "alpha"));
    lok(s != NULL);

    for (int i = 0; i < expectsz; i++) {
        lequal((int)expect[i], (int)C_Wstring_char_at(s, i));
    }
    lequal(0, (int)C_Wstring_char_at(s, expectsz));
    lequal(expectsz, (int)C_Wstring_length(s));

    lok(C_Wstring_release(&s));
    lok(s == NULL);
}

/*
 * TODO: test
USTR_API bool C_Wstring_new_ascii(const C_Wstring* *sp, size_t sz, const char* buf);
*/

static void string_from_utf8() {
    wchar_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Wstring* s = NULL;
        const utf8_t* str = wcs2utf8(EXPECT[i].expect);
        int len = strlen(str);

        lok(C_Wstring_new_utf8(&s, len, str));
        lok(s != NULL);

        if (s) {
            int result = (int)C_Wstring_to_utf32(s, 0, STRBUFSIZ, buffer);
            lequal((int) wcslen(EXPECT[i].expect) + 1, result);
            lsequal(wcs2cstr(EXPECT[i].expect), wcs2cstr(buffer));
        }
        lok(C_Wstring_release(&s));
    }

    free_strings();
}

static void string_from_utf16() {
    wchar_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Wstring* s = NULL;
        const utf16_t* jstr = wcs2utf16(EXPECT[i].expect);
        int jlen = jcslen(jstr);

        lok(C_Wstring_new_utf16(&s, jlen, jstr));
        lok(s != NULL);

        if (s) {
            int result = (int)C_Wstring_to_utf32(s, 0, STRBUFSIZ, buffer);
            lequal((int) wcslen(EXPECT[i].expect) + 1, result);
            lsequal(wcs2cstr(EXPECT[i].expect), wcs2cstr(buffer));
        }
        lok(C_Wstring_release(&s));
    }

    free_strings();
}

static void string_from_utf32() {
    wchar_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Wstring* s = NULL;
        const wchar_t* wstr = EXPECT[i].expect;
        int wlen = wcslen(wstr);

        lok(C_Wstring_new_utf32(&s, wlen, wstr));
        lok(s != NULL);

        if (s) {
            int result = (int)C_Wstring_to_utf32(s, 0, STRBUFSIZ, buffer);
            lequal((int) wcslen(EXPECT[i].expect) + 1, result);
            lsequal(wcs2cstr(EXPECT[i].expect), wcs2cstr(buffer));
        }
        lok(C_Wstring_release(&s));
    }

    free_strings();
}

static void string_from_charset() {
    wchar_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));
    wmemset(buffer, '!', STRBUFSIZ-1);

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Wstring* s = NULL;
        const char* cs = EXPECT[i].charset;
        const char* instr = EXPECT[i].input;
        int inlen = EXPECT[i].len;
        const wchar_t* expstr = EXPECT[i].expect;
        int explen = wcslen(expstr);

        lok(C_Wstring_new_encoded(&s, cs, inlen, (const octet_t*)instr));
        lok(s != NULL);

        if (s) {
            lequal(explen+1, (int)C_Wstring_to_utf32(s, 0, STRBUFSIZ, buffer));
            lsequal(wcs2cstr(expstr), wcs2cstr(buffer));
        }
        lok(C_Wstring_release(&s));
    }

    free_strings();
}

static void string_to_utf8() {
    utf8_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        const wchar_t* wstr = EXPECT[i].expect;
        const int wlen = wcslen(wstr);
        const utf8_t* expstr = wcs2utf8(wstr);
        const int explen = strlen(expstr);
        const C_Wstring* s = NULL;

        lok(C_Wstring_new_utf32(&s, wlen, wstr));
        lok(s != NULL);

        if (s) {
            size_t result = C_Wstring_to_utf8(s, 0, STRBUFSIZ, buffer);
            lequal(explen + 1, (int) result);
            lsequal(expstr, (const char*)buffer);
        }
        lok(C_Wstring_release(&s));
    }

    free_strings();
}

static void string_to_charset() {
    utf8_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        const wchar_t* wstr = EXPECT[i].expect;
        const int wlen = wcslen(wstr);
        const utf8_t* expstr = wcs2utf8(wstr);
        const int explen = strlen(expstr);
        const C_Wstring* s = NULL;

        lok(C_Wstring_new_utf32(&s, wlen, wstr));
        lok(s != NULL);

        if (s) {
            size_t result = 
                    C_Wstring_to_charset(s, UTF_8, 0, STRBUFSIZ, buffer);
            lequal(explen + 1, (int)result);
            lsequal(expstr, (const char*)buffer);
        }
        lok(C_Wstring_release(&s));
    }

    free_strings();
}

static void string_equals() {
    const C_Wstring* s1a;
    const C_Wstring* s1b;
    const C_Wstring* s2;
    const C_Wstring* s3a;
    const C_Wstring* s3b;

    lok(C_Wstring_new_from_cstring(&s1a, "test1"));
    lok(C_Wstring_new_from_cstring(&s1b, "test1"));
    lok(C_Wstring_new_from_cstring(&s2, "test2"));
    lok(C_Wstring_new_from_cstring(&s3a, ""));
    lok(C_Wstring_new_from_cstring(&s3b, ""));

    lequal(true, C_Wstring_equals(s1a, s1b));
    lok(C_Wstring_compare(s1a, s1b) == 0);
    lok(C_Wstring_hashcode(s1a) == C_Wstring_hashcode(s1b));

    lequal(false, C_Wstring_equals(s1a, s2));
    lok(C_Wstring_compare(s1a, s2) < 0);
    lequal(false, C_Wstring_equals(s1a, s3a));
    lok(C_Wstring_compare(s1a, s3a) > 0);
    lequal(false, C_Wstring_equals(s1a, NULL));
    lequal(false, C_Wstring_equals(NULL, s2));

    lequal(true, C_Wstring_equals(NULL, NULL));
    lok(C_Wstring_hashcode(NULL) == 0);

    lequal(true, C_Wstring_equals(s2, s2));
    lok(C_Wstring_hashcode(s2) == C_Wstring_hashcode(s2));

    lequal(true, C_Wstring_equals(s3a, s3b));
    lok(C_Wstring_hashcode(s3a) == C_Wstring_hashcode(s3a));
}

/*
 * TODO: test:
 *
USTR_API bool C_Wstring_slice(const C_Wstring* *sp, const C_Wstring* s, int first, int last);
USTR_API bool C_Wstring_slice_from(const C_Wstring* *sp, const C_Wstring* s, int first);
USTR_API bool C_Wstring_slice_to(const C_Wstring* *sp, const C_Wstring* s, int last);

USTR_API bool C_Wstring_join(const C_Wstring* *sp, const C_Wstring* head, const C_Wstring* tail);
USTR_API bool C_Wstring_join_n(const C_Wstring* *sp, size_t n, ...);
*/

int main (int argc, char* argv[]) {
    lrun("string_smoke", string_smoke);
    lrun("string_chars", string_chars);
    lrun("string_from_utf8", string_from_utf8);
    lrun("string_from_utf16", string_from_utf16);
    lrun("string_from_utf32", string_from_utf32);
    lrun("string_from_charset", string_from_charset);
    lrun("string_to_utf8", string_to_utf8);
    lrun("string_to_charset", string_to_charset);
    lrun("string_equals", string_equals);
    lresults();
    return lfails != 0;
}
