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
#include "cconv.h"
#include "ustring.h"

#define STRBUFSIZ   64

/* ------------------------ TEST DATA ----------------------------- */

typedef struct string_data {
    const size_t   len;
    const char*    input;
    const char*    encoding;
    const wchar_t* expect;
} string_data;

string_data EXPECT[] = {
    {  5, "alpha",              "US-ASCII",     L"alpha"},
    {  4, "beta",               "US-ASCII",     L"beta"},
    {  5, "gamma",              "ASCII",        L"gamma"},
    {  5, "delta",              "US-ASCII",     L"delta"},
    { 15, "verisimilitude",     "US-ASCII",     L"verisimilitude"},
    {  6, "tsch\xfc\xdf",       "LATIN1",       L"tschüß"},
    {  4, "\x20\xAC\x00?",      "UCS-2BE",      L"\u20AC?"},
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

static const char* wcs2utf8(const wchar_t* wcs) {
    char buf[STRBUFSIZ+2];
    size_t wlen = wcslen(wcs);
    size_t len;

    bzero(buf, sizeof(buf));
    len = C_Conv_utf32_to_8(wlen, wcs, STRBUFSIZ, buf);
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

/* ------------------------------ TESTS ----------------------------- */

static void string_smoke() {
    U_String* s = NULL;
    uint8_t outbuf[STRBUFSIZ];

    lok(U_String_new_from_cstring(&s, "alpha"));
    lok(s != NULL);

    lok(U_String_release(&s));
    lok(s == NULL);
}

static void string_chars() {
    U_String* s = NULL;
    const wchar_t* expect = L"alpha";
    int expectsz = wcslen(expect);
    uint8_t outbuf[STRBUFSIZ];

    lok(U_String_new_from_cstring(&s, "alpha"));
    lok(s != NULL);

    for (int i = 0; i < expectsz; i++) {
        lequal((int)expect[i], (int)U_String_char_at(s, i));
    }
    lequal(0, (int)U_String_char_at(s, expectsz));
    lequal(expectsz, (int)U_String_length(s));

    lok(U_String_release(&s));
    lok(s == NULL);
}

static void string_encoding() {
    wchar_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));
    wmemset(buffer, '!', STRBUFSIZ-1);

    for (int i = 0; i < EXPECTSZ; i++) {
        U_String* s = NULL;
        C_Symbol* e = NULL;
        int len = EXPECT[i].len;

        C_Symbol_for_cstring(&e, EXPECT[i].encoding);
        lok(e != NULL);

        lok(U_String_new_encoded(&s, e, len, (const octet_t*)EXPECT[i].input));
        lok(s != NULL);

        if (s) {
            lok(U_String_to_utf32(s, 0, STRBUFSIZ, buffer));
            lsequal(wcs2cstr(EXPECT[i].expect), wcs2cstr(buffer));
        }
    }

    free_strings();
}

static void string_encoding_utf8() {
    utf8_t buffer[STRBUFSIZ];

    bzero(buffer, sizeof(buffer));

    for (int i = 0; i < EXPECTSZ; i++) {
        U_String* s = NULL;
        C_Symbol* e = NULL;
        int len = EXPECT[i].len;

        C_Symbol_for_cstring(&e, EXPECT[i].encoding);
        lok(e != NULL);

        lok(U_String_new_encoded(&s, e, len, (const octet_t*)EXPECT[i].input));
        lok(s != NULL);

        if (s) {
            lok(U_String_to_utf8(s, 0, STRBUFSIZ, buffer));
            lsequal(wcs2utf8(EXPECT[i].expect), (const char*)buffer);
        }
    }

    free_strings();
}

/*
USTR_API void U_String_set_allocator(u_string_alloc a, void *data);

USTR_API bool U_String_new_ascii(U_String* *sp, size_t sz, const char* buf);

USTR_API bool U_String_new_utf8(U_String* *sp, size_t sz, const utf8_t* buf);

USTR_API bool U_String_new_utf16(U_String* *sp, size_t sz, const utf16_t* buf);

USTR_API bool U_String_new_utf32(U_String* *sp, size_t sz, const wchar_t* buf);

USTR_API size_t U_String_each(U_String* s, void* data, u_iterator f);
USTR_API size_t U_String_each_after(U_String* s, size_t index, void* data, u_iterator f);

USTR_API bool U_String_slice(U_String* *sp, U_String* s, int first, int last);
USTR_API bool U_String_slice_from(U_String* *sp, U_String* s, int first);
USTR_API bool U_String_slice_to(U_String* *sp, U_String* s, int last);

USTR_API bool U_String_join(U_String* *sp, U_String* head, U_String* tail);
USTR_API bool U_String_join_n(U_String* *sp, size_t n, ...);

USTR_API U_String* U_String_set(U_String* *lvalue, U_String* rvalue);
*/

int main (int argc, char* argv[]) {
    lrun("string_smoke", string_smoke);
    lrun("string_chars", string_chars);
    lrun("string_encoding", string_encoding);
    lrun("string_encoding_utf8", string_encoding_utf8);
    lresults();
    return lfails != 0;
}
