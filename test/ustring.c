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
#include "minctest.h"
#include "convert.h"
#include "ustring.h"

#define STRBUFSIZ       256
#define JSTRBUFSIZ      128
#define USTRBUFSIZ      64

/* ------------------------ TEST DATA ----------------------------- */

typedef struct string_data {
    const size_t   len;
    const char*    input;
    const char*    charset;
    const char32_t* expect;
} string_data;

string_data EXPECT[] = {
    {  0, "",                   "LATIN1",       U""},
    {  5, "alpha",              "US-ASCII",     U"alpha"},
    {  4, "beta",               "US-ASCII",     U"beta"},
    {  5, "gamma",              "ASCII",        U"gamma"},
    {  5, "delta",              "US-ASCII",     U"delta"},
    { 14, "verisimilitude",     "US-ASCII",     U"verisimilitude"},
    {  6, "tsch\xfc\xdf",       "LATIN1",       U"tschüß"}
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

static const size_t jstrlen(const char16_t* jstr) {
    int i = 0;
    while (jstr[i] != 0) {
        i++;
    }
    return i;
}

static const size_t ucslen(const char32_t* ucs) {
    int i = 0;
    while (ucs[i] != 0) {
        i++;
    }
    return i;
}

static const char* ucs2cstr(const char32_t* ucs) {
    char buf[STRBUFSIZ];
    size_t ulen = ucslen(ucs);
    size_t len = 0;

    memset(buf, 0, sizeof(buf));
    for (int i = 0; i < ulen && len + 4 < STRBUFSIZ; i++) {
        len = append_ascii(ucs[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static const char* jstr2cstr(const char16_t* jstr) {
    char buf[STRBUFSIZ];
    size_t jlen = jstrlen(jstr);
    size_t len = 0;

    memset(buf, 0, sizeof(buf));
    for (int i = 0; i < jlen && len + 4 < STRBUFSIZ; i++) {
        len = append_ascii(jstr[i], buf, len);
    }
    return (char*)stralloc(buf, len, sizeof(char));
}

static const char8_t* ucs2utf8(const char32_t* ucs) {
    char8_t buf[STRBUFSIZ+2];
    size_t ulen = ucslen(ucs);
    size_t len;

    memset(buf, 0, sizeof(buf));
    len = C_Conv_char32_to_8(ulen, ucs, STRBUFSIZ, buf);
    return (char8_t*)stralloc(buf, len, sizeof(char));
}

static const char16_t* ucs2utf16(const char32_t* ucs) {
    char16_t buf[JSTRBUFSIZ];
    size_t ulen = ucslen(ucs);
    size_t len;

    memset(buf, 0, sizeof(buf));
    len = C_Conv_char32_to_16(ulen, ucs, JSTRBUFSIZ, buf);
    return (char16_t*)stralloc(buf, len, sizeof(char16_t));
}

static const size_t jcslen(const char16_t* jcs) {
    int i = 0;
    while (jcs[i] != 0) {
        i++;
    }
    return i;
}

static const char32_t* cstr2ucs(const char* s) {
    char32_t buf[USTRBUFSIZ];
    size_t len = strlen(s);
    int i;

    memset(buf, 0, sizeof(buf));
    for (i = 0; i < len; i++) {
        buf[i] = (char32_t)s[i];
    }
    lequal((int)len, i);
    return (char32_t*)stralloc(buf, len, sizeof(char32_t));
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
    const C_Ustring* s = NULL;

    lok(C_Ustring_new_from_cstring(&s, "alpha"));
    lok(s != NULL);

    lok(C_Ustring_release(&s));
    lok(s == NULL);
}

static void string_chars() {
    const C_Ustring* s = NULL;
    const char32_t* expect = U"alpha";
    int expectsz = ucslen(expect);

    lok(C_Ustring_new_from_cstring(&s, "alpha"));
    lok(s != NULL);

    for (int i = 0; i < expectsz; i++) {
        lequal((int)expect[i], (int)C_Ustring_char_at(s, i));
    }
    lequal(0, (int)C_Ustring_char_at(s, expectsz));
    lequal(expectsz, (int)C_Ustring_length(s));

    lok(C_Ustring_release(&s));
    lok(s == NULL);
}

static void string_from_utf8() {
    char32_t buffer[USTRBUFSIZ];

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Ustring* s = NULL;
        const char8_t* str = ucs2utf8(EXPECT[i].expect);
        int len = strlen(str);

        lok(C_Ustring_new_utf8(&s, len, str));
        lok(s != NULL);

        if (s) {
            memset(buffer, 0, sizeof(buffer));

            int result = (int)C_Ustring_to_utf32(s, 0, USTRBUFSIZ, buffer);
            lequal((int) ucslen(EXPECT[i].expect) + 1, result);
            lsequal(ucs2cstr(EXPECT[i].expect), ucs2cstr(buffer));
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_from_utf16() {
    char32_t buffer[USTRBUFSIZ];

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Ustring* s = NULL;
        const char16_t* jstr = ucs2utf16(EXPECT[i].expect);
        int jlen = jcslen(jstr);

        lok(C_Ustring_new_utf16(&s, jlen, jstr));
        lok(s != NULL);

        if (s) {
            memset(buffer, 0, sizeof(buffer));

            int result = (int)C_Ustring_to_utf32(s, 0, USTRBUFSIZ, buffer);
            lequal((int) ucslen(EXPECT[i].expect) + 1, result);
            lsequal(ucs2cstr(EXPECT[i].expect), ucs2cstr(buffer));
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_from_utf32() {
    char32_t buffer[USTRBUFSIZ];

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Ustring* s = NULL;
        const char32_t* ustr = EXPECT[i].expect;
        int ulen = ucslen(ustr);

        lok(C_Ustring_new_utf32(&s, ulen, ustr));
        lok(s != NULL);

        if (s) {
            memset(buffer, 0, sizeof(buffer));

            int result = (int)C_Ustring_to_utf32(s, 0, USTRBUFSIZ, buffer);
            lequal((int) ucslen(EXPECT[i].expect) + 1, result);
            lsequal(ucs2cstr(EXPECT[i].expect), ucs2cstr(buffer));
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_from_charset() {
    char32_t buffer[USTRBUFSIZ];

    for (int i = 0; i < EXPECTSZ; i++) {
        const C_Ustring* s = NULL;
        const char* cs = EXPECT[i].charset;
        const char* instr = EXPECT[i].input;
        int inlen = EXPECT[i].len;
        const char32_t* expstr = EXPECT[i].expect;
        int explen = ucslen(expstr);

        lok(C_Ustring_new_encoded(&s, cs, inlen, (const octet_t*)instr));
        lok(s != NULL);

        if (s) {
            memset(buffer, 0, sizeof(buffer));

            ssize_t result =
                C_Ustring_to_utf32(s, 0, USTRBUFSIZ, buffer);

            lequal(explen+1, (int)result);
            lsequal(ucs2cstr(expstr), ucs2cstr(buffer));
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_to_utf8() {
    char8_t buffer[STRBUFSIZ];

    for (int i = 0; i < EXPECTSZ; i++) {
        const char32_t* ustr = EXPECT[i].expect;
        const int ulen = ucslen(ustr);
        const char8_t* expstr = ucs2utf8(ustr);
        const int explen = strlen(expstr);
        const C_Ustring* s = NULL;

        lok(C_Ustring_new_utf32(&s, ulen, ustr));
        lok(s != NULL);

        if (s) {
            memset(buffer, 0, sizeof(buffer));

            size_t result = C_Ustring_to_utf8(s, 0, STRBUFSIZ, buffer);
            lequal(explen + 1, (int) result);
            lsequal(expstr, (const char*)buffer);
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_to_utf16() {
    char16_t buffer[JSTRBUFSIZ];

    for (int i = 0; i < EXPECTSZ; i++) {
        const char32_t* ustr = EXPECT[i].expect;
        const int ulen = ucslen(ustr);
        const char16_t* expstr = ucs2utf16(ustr);
        const int explen = jstrlen(expstr) * sizeof(char16_t);
        const C_Ustring* s = NULL;

        lok(C_Ustring_new_utf32(&s, ulen, ustr));
        lok(s != NULL);

        if (s) {
            memset(buffer, 0, sizeof(buffer));

            size_t result = 
                    C_Ustring_to_charset(s, UTF_16, 0, JSTRBUFSIZ, (octet_t*)buffer);
            lequal(explen + 2, (int)result);
            lsequal(jstr2cstr(expstr), jstr2cstr(buffer));
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_to_utf32() {
    char32_t buffer[USTRBUFSIZ];

    memset(buffer, 0, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        const char32_t* ustr = EXPECT[i].expect;
        const int ulen = ucslen(ustr);
        const char32_t* expstr = ustr;
        const int explen = ulen * sizeof(char32_t);
        const C_Ustring* s = NULL;

        lok(C_Ustring_new_utf32(&s, ulen, ustr));
        lok(s != NULL);

        if (s) {
            size_t result = 
                    C_Ustring_to_charset(s, UTF_32, 0, USTRBUFSIZ, (octet_t*)buffer);
            lequal(explen + 4, (int)result);
            lsequal(ucs2cstr(expstr), ucs2cstr(buffer));
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_to_charset() {
    char8_t buffer[STRBUFSIZ];

    memset(buffer, 0, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        const char32_t* ustr = EXPECT[i].expect;
        const int ulen = ucslen(ustr);
        const char8_t* expstr = ucs2utf8(ustr);
        const int explen = strlen(expstr);
        const C_Ustring* s = NULL;

        lok(C_Ustring_new_utf32(&s, ulen, ustr));
        lok(s != NULL);

        if (s) {
            size_t result =
                    C_Ustring_to_charset(s, UTF_8, 0, STRBUFSIZ, buffer);
            lequal(explen + 1, (int)result);
            lsequal(expstr, (const char*)buffer);
        }
        lok(C_Ustring_release(&s));
    }

    free_strings();
}

static void string_equals() {
    const C_Ustring* s1a;
    const C_Ustring* s1b;
    const C_Ustring* s2;
    const C_Ustring* s3a;
    const C_Ustring* s3b;

    lok(C_Ustring_new_from_cstring(&s1a, "test1"));
    lok(C_Ustring_new_from_cstring(&s1b, "test1"));
    lok(C_Ustring_new_from_cstring(&s2, "test2"));
    lok(C_Ustring_new_from_cstring(&s3a, ""));
    lok(C_Ustring_new_from_cstring(&s3b, ""));

    lequal(true, C_Ustring_equals(s1a, s1b));
    lok(C_Ustring_compare(s1a, s1b) == 0);
    lok(C_Ustring_hashcode(s1a) == C_Ustring_hashcode(s1b));

    lequal(false, C_Ustring_equals(s1a, s2));
    lok(C_Ustring_compare(s1a, s2) < 0);
    lequal(false, C_Ustring_equals(s1a, s3a));
    lok(C_Ustring_compare(s1a, s3a) > 0);
    lequal(false, C_Ustring_equals(s1a, NULL));
    lequal(false, C_Ustring_equals(NULL, s2));

    lequal(true, C_Ustring_equals(NULL, NULL));
    lok(C_Ustring_hashcode(NULL) == 0);

    lequal(true, C_Ustring_equals(s2, s2));
    lok(C_Ustring_hashcode(s2) == C_Ustring_hashcode(s2));

    lequal(true, C_Ustring_equals(s3a, s3b));
    lok(C_Ustring_hashcode(s3a) == C_Ustring_hashcode(s3a));
}

/*
 * TODO: test:
 *
USTR_API bool C_Ustring_new_ascii(const C_Ustring* *sp, size_t sz, const char* buf);

USTR_API bool C_Ustring_slice(const C_Ustring* *sp, const C_Ustring* s, int first, int last);
USTR_API bool C_Ustring_slice_from(const C_Ustring* *sp, const C_Ustring* s, int first);
USTR_API bool C_Ustring_slice_to(const C_Ustring* *sp, const C_Ustring* s, int last);

USTR_API bool C_Ustring_join(const C_Ustring* *sp, const C_Ustring* head, const C_Ustring* tail);
USTR_API bool C_Ustring_join_n(const C_Ustring* *sp, size_t n, ...);
*/

int main (int argc, char* argv[]) {
    lrun("string_smoke", string_smoke);
    lrun("string_chars", string_chars);
    lrun("string_from_utf8", string_from_utf8);
    lrun("string_from_utf16", string_from_utf16);
    lrun("string_from_utf32", string_from_utf32);
    lrun("string_from_charset", string_from_charset);
    lrun("string_to_utf8", string_to_utf8);
    lrun("string_to_utf16", string_to_utf16);
    lrun("string_to_utf32", string_to_utf32);
    lrun("string_to_charset", string_to_charset);
    lrun("string_equals", string_equals);
    lresults();
    return lfails != 0;
}
