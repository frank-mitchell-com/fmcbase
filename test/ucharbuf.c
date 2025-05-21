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
#include "ucharbuf.h"

#define STRBUFSIZ   64

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

static size_t ucslen(const char32_t* ucs) {
    size_t result = 0;
    while (ucs[result] != 0) {
        result++;
    }
    return result;
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

static void charbuf_smoke() {
    C_Uchar_Buffer* b = NULL;
    C_Uchar_Buffer* oldb = NULL;

    C_Uchar_Buffer_new(&b);
    lok(b != NULL);
    lequal(0, (int)C_Uchar_Buffer_length(b));

    oldb = b;
    C_Uchar_Buffer_release(&b);
    lequal(false, C_Uchar_Buffer_is_live(oldb));
}

static void charbuf_chars() {
    C_Uchar_Buffer* b = NULL;
    const C_Ustring* s = NULL;
    const char32_t* ucs = U"test";
    const size_t wlen = ucslen(ucs);

    C_Ustring_new_utf32(&s, wlen, ucs);
    lok(s != NULL);
    if (s != NULL) {
        C_Uchar_Buffer_new_from_string(&b, s);
        lok(b != NULL);
        lequal((int)wlen, (int)C_Uchar_Buffer_length(b));

        for (int i = 0; i < wlen; i++) {
            lequal((int)ucs[i], (int)C_Uchar_Buffer_char_at(b, i));
            lequal((int)ucs[i], (int)C_Uchar_Buffer_char_at(b, -(wlen-i)));
        }
    }

    C_Ustring_release(&s);
    C_Uchar_Buffer_release(&b);
}

static void charbuf_ucs() {
    C_Uchar_Buffer* b = NULL;
    const char32_t* ucs = U"test";
    const size_t wlen = ucslen(ucs);

    C_Uchar_Buffer_new_from_ucs(&b, ucs);
    lok(b != NULL);
    lequal((int)wlen, (int)C_Uchar_Buffer_length(b));

    for (int i = 0; i < wlen; i++) {
        lequal((int)ucs[i], (int)C_Uchar_Buffer_char_at(b, i));
    }

    C_Uchar_Buffer_release(&b);
}

int main (int argc, char* argv[]) {
    lrun("charbuf_smoke", charbuf_smoke);
    lrun("charbuf_chars", charbuf_chars);
    lrun("charbuf_ucs", charbuf_ucs);
    lresults();
    return lfails != 0;
}
