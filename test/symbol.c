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
#include <strings.h>
#include "minctest.h"
#include "symbol.h"

#define STRBUFSIZ   20

static void symbol_new() {
    C_Symbol* sym;

    C_Symbol_new(&sym);
    lok(sym != NULL);
    lok(is_C_Symbol(sym));

    C_Symbol_release(&sym);
    lok(sym == NULL);
}

static void symbol_unique() {
    C_Symbol* sym1;
    C_Symbol* sym2;

    C_Symbol_new(&sym1);
    lok(sym1 != NULL);
    lok(is_C_Symbol(sym1));

    C_Symbol_new(&sym2);
    lok(sym2 != NULL);
    lok(is_C_Symbol(sym2));

    lok(sym1 != sym2);

    C_Symbol_release(&sym1);
    C_Symbol_release(&sym2);
}

static void symbol_retain() {
    C_Symbol* sym1;
    C_Symbol* sym2;

    C_Symbol_new(&sym1);
    lok(sym1 != NULL);
    lok(is_C_Symbol(sym1));

    sym2 = C_Symbol_retain(sym1);
    lok(sym2 == sym1);
    lequal(2, C_Symbol_references(sym1));

    C_Symbol_release(&sym2);
    lequal(is_C_Symbol(sym1), true);
    lequal(1, C_Symbol_references(sym1));

    C_Symbol_release(&sym1);
}

static void symbol_set() {
    C_Symbol* sym1;
    C_Symbol* sym2;

    C_Symbol_new(&sym1);
    lok(sym1 != NULL);
    lok(is_C_Symbol(sym1));
    lequal(1, C_Symbol_references(sym1));

    C_Symbol_set(&sym2, sym1);

    lok(sym1 == sym2);
    lok(is_C_Symbol(sym1));
    lok(is_C_Symbol(sym2));
    lequal(2, C_Symbol_references(sym1));
    lequal(2, C_Symbol_references(sym2));

    C_Symbol_release(&sym1);
    C_Symbol_release(&sym2);
}

static void symbol_set_over() {
    C_Symbol* sym1;
    C_Symbol* sym2;
    C_Symbol* oldsym2;

    C_Symbol_new(&sym1);
    lok(sym1 != NULL);
    lok(is_C_Symbol(sym1));
    lequal(1, C_Symbol_references(sym1));

    C_Symbol_new(&sym2);
    lok(sym2 != NULL);
    lok(is_C_Symbol(sym2));
    lequal(1, C_Symbol_references(sym2));

    oldsym2 = sym2; // Never do this
    C_Symbol_set(&sym2, sym1);
    lequal(is_C_Symbol(oldsym2), false);

    lok(sym1 == sym2);
    lok(is_C_Symbol(sym1));
    lequal(2, C_Symbol_references(sym1));
    lequal(2, C_Symbol_references(sym2));

    C_Symbol_release(&sym2);
    lequal(is_C_Symbol(sym1), true);
    lequal(1, C_Symbol_references(sym1));

    C_Symbol_release(&sym1);
}

static void symbol_set_itself() {
    C_Symbol* sym;

    C_Symbol_new(&sym);
    lok(sym != NULL);
    lok(is_C_Symbol(sym));
    lequal(1, C_Symbol_references(sym));

    C_Symbol_set(&sym, sym);

    lequal(1, C_Symbol_references(sym));

    C_Symbol_release(&sym);
}

static void symbol_for_cstring() {
    C_Symbol* sym1;
    C_Symbol* sym2;
    ssize_t length;
    char buffer[STRBUFSIZ];

    C_Symbol_for_cstring(&sym1, "cstring");
    lok(sym1 != NULL);
    lok(is_C_Symbol(sym1));

    C_Symbol_for_cstring(&sym2, "cstring");
    lok(sym2 != NULL);
    lok(is_C_Symbol(sym2));

    lok(sym1 == sym2);

    bzero(buffer, STRBUFSIZ);
    length = C_Symbol_as_cstring(sym1, STRBUFSIZ, buffer);
    lok(length == 7);
    lsequal("cstring", buffer);

    bzero(buffer, STRBUFSIZ);
    length = C_Symbol_as_cstring(sym2, STRBUFSIZ, buffer);
    lok(length == 7);
    lsequal("cstring", buffer);

    C_Symbol_release(&sym1);
    C_Symbol_release(&sym2);
    lok(sym1 == NULL);
    lok(sym2 == NULL);
}

static void symbol_as_cstring() {
    C_Symbol* strsym;
    C_Symbol* sym;
    C_Symbol* notsym;
    ssize_t length;
    char buffer[STRBUFSIZ];

    C_Symbol_for_cstring(&strsym, "ISO-Latin-1");
    lok(strsym != NULL);
    lok(is_C_Symbol(strsym));

    bzero(buffer, STRBUFSIZ);
    length = C_Symbol_as_cstring(strsym, STRBUFSIZ, buffer);
    lok(length == 11);
    lsequal("ISO-Latin-1", buffer);

    C_Symbol_new(&sym);
    lok(sym != NULL);
    lok(is_C_Symbol(sym));

    bzero(buffer, STRBUFSIZ);
    length = C_Symbol_as_cstring(sym, STRBUFSIZ, buffer);
    lok(length == 0);
    lsequal("", buffer);

    notsym = sym; // Never do this

    C_Symbol_release(&sym);
    lok(sym == NULL);

    bzero(buffer, STRBUFSIZ);
    length = C_Symbol_as_cstring(sym, STRBUFSIZ, buffer);
    lok(length == -1);
}

static void symbol_as_utf8_string() {
    C_Symbol* strsym;
    C_Symbol* sym;
    C_Symbol* notsym;
    size_t length;
    const char* actual;

    C_Symbol_for_cstring(&strsym, "ISO-Latin-2");
    lok(strsym != NULL);
    lok(is_C_Symbol(strsym));

    actual = (const char *)C_Symbol_as_utf8_string(strsym, &length);
    lok(length == 11);
    lsequal("ISO-Latin-2", actual);

    C_Symbol_new(&sym);
    lok(sym != NULL);
    lok(is_C_Symbol(sym));

    actual = (const char *)C_Symbol_as_utf8_string(sym, &length);
    lok(length == 0);
    lsequal("", actual);

    notsym = sym; // Never do this

    C_Symbol_release(&sym);
    lok(sym == NULL);

    actual = (const char *)C_Symbol_as_utf8_string(notsym, &length);
    lok(actual == NULL);
}

int main (int argc, char* argv[]) {
    lrun("symbol_new", symbol_new);
    lrun("symbol_unique", symbol_unique);
    lrun("symbol_retain", symbol_retain);
    lrun("symbol_set", symbol_set);
    lrun("symbol_set_over", symbol_set_over);
    lrun("symbol_set_itself", symbol_set_itself);
    lrun("symbol_for_cstring", symbol_for_cstring);
    lrun("symbol_as_cstring", symbol_as_cstring);
    lrun("symbol_as_utf8_string", symbol_as_utf8_string);
    lresults();
    return lfails != 0;
}
