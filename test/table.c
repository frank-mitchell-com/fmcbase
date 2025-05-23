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
#include <ctype.h>
#include "minctest.h"
#include "table.h"

#define STRBUFSIZ  512 

static C_Table* t = NULL;

typedef struct _kvpair {
    const char* key; 
    const char* value;
} kvpair;

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
    if (isprint(c) || c == ' ') {
        buf[j] = (char)c;
        len += 1;
    } else {
        sprintf(&(buf[len]), "\\u{%x}", c); 
        len = strlen(buf);
    }
    return len;
}

static const char* tmpstr(const char* instr) {
    char buf[STRBUFSIZ];
    size_t inlen = strlen(instr);
    size_t len = 0;

    memset(buf, 0, STRBUFSIZ);
    for (int i = 0; i < inlen; i++) {
        len = append_ascii(instr[i], buf, len);
    }
    return (const char*)stralloc(buf, len, sizeof(char));
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

static const char* tostr(C_Userdata ud) {
    char buf[STRBUFSIZ];

    if (ud.len == 0 || ud.ptr == NULL) {
        sprintf(buf, "{tag=%d,len=%ld,ptr=%p}", ud.tag, ud.len, ud.ptr);
    } else {
        sprintf(buf, "{tag=%d,len=%ld,ptr=\"%*s\"}", ud.tag, ud.len, (int)ud.len, (char*)ud.ptr);
    }
    return tmpstr(buf);
}

static void setup() {
    t = NULL;
    C_Table_new(&t, 3);
    lok(t != NULL);
}

static void teardown() {
    C_Table_free(&t);
    free_strings();
}

static void table_smoke() {
    setup();

    teardown();
}

static void table_has() {
    C_Userdata key;

    setup();

    C_Userdata_set_string(&key, "key");

    lequal(C_Table_has(t, &key), false);

    teardown();
}

static void table_get() {
    C_Userdata key, value;

    setup();

    C_Userdata_set_string(&key, "key");
    C_Userdata_clear(&value, false);

    lequal(C_Table_get(t, &key, &value), false);

    teardown();
}

static void table_add() {
    C_Userdata key, value, value2, actual;

    setup();

    C_Userdata_set_string(&key, "key");
    C_Userdata_set_string(&value, "value");
    C_Userdata_set_string(&value2, "value2");

    // Check key not in table
    lequal(C_Table_has(t, &key), false);

    // ADD key = value
    lok(C_Table_add(t, &key, &value));

    // Check key = value in table
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value), tostr(actual));
    lequal(1, (int)C_Table_size(t));

    // Try to add the key again
    lequal(C_Table_add(t, &key, &value2), false);

    // Check key = value in table
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value), tostr(actual));
    lequal(1, (int)C_Table_size(t));

    teardown();
}

static void table_add_multiple() {
    C_Userdata key, value, actual;
    kvpair expected[] = {
        { "alpha",   "alpha" },
        { "bravo",   "bravo" },
        { "charlie", "charlie" },
        { "delta",   "delta" },
        { "echo",    "echo" },
        { "foxtrot", "foxtrot" },
        { "golf",    "golf" },
        { "hotel",   "hotel" },
        { "india",   "india" },
        { "juliet",  "juliet" },
        { "kilo",    "kilo" },
        { "lima",    "lima" },
        { "mike",    "mike" },
        { "Takehito", "Koyasu" },
        { "Saori",    "Hayami" },
        { "Akio",     "Ootori" },
        { NULL, NULL }
    };

    setup();

    for (int i = 0; expected[i].key != NULL; i++) {
        // ADD key = value
        C_Userdata_set_string(&key, expected[i].key);
        C_Userdata_set_string(&value, expected[i].value);
        lok(C_Table_add(t, &key, &value));

        // Check key = value in table
        C_Userdata_clear(&actual, false);
        lok(C_Table_get(t, &key, &actual));
        lsequal(tostr(value), tostr(actual));
    }

    lequal(16, (int)C_Table_size(t));

    for (int i = 0; expected[i].key != NULL; i++) {
        // Check key = value in table AGAIN
        C_Userdata_set_string(&key, expected[i].key);
        C_Userdata_clear(&actual, false);
        lok(C_Table_get(t, &key, &actual));
        lsequal(expected[i].value, (char*)actual.ptr);
    }

    lequal(16, (int)C_Table_size(t));

    teardown();
}

static void table_with_pointer_key() {
    char *data = (char *)calloc(50, sizeof(char));

    C_Userdata key, value, actual;

    setup();

    strcpy(data, "this is my data");

    C_Userdata_set_pointer(&key, data);
    C_Userdata_set_string(&value, "value");

    // Check key not in table
    lequal(C_Table_has(t, &key), false);
    lequal(0, (int)C_Table_size(t));

    // ADD key = value
    lok(C_Table_add(t, &key, &value));

    // Check key in table
    lequal(C_Table_has(t, &key), true);
    lequal(1, (int)C_Table_size(t));

    strcpy(data, "still my data");

    // Check key still in table even though contents changed
    // i.e. stored by reference, not value.
    lequal(C_Table_has(t, &key), true);

    teardown();
}

static void table_with_pointer_value() {
    const char *data = strdup("this is my data");

    C_Userdata key, value, actual;

    setup();

    C_Userdata_set_string(&key, "key");
    C_Userdata_set_pointer(&value, data);

    // Check key not in table
    lequal(C_Table_has(t, &key), false);
    lequal(0, (int)C_Table_size(t));

    // ADD key = value
    lok(C_Table_add(t, &key, &value));

    // Check key = value in table
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value), tostr(actual));
    lok(data == actual.ptr);
    lequal(1, (int)C_Table_size(t));

    teardown();
}

static void table_put() {
    C_Userdata key, value, value2, actual;

    setup();

    C_Userdata_set_string(&key, "key");
    C_Userdata_set_string(&value, "value");
    C_Userdata_set_string(&value2, "value2");

    // Check key not in table
    lequal(C_Table_has(t, &key), false);
    lequal(0, (int)C_Table_size(t));

    // PUT key = value
    lok(C_Table_put(t, &key, &value));

    // Check key = value
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value), tostr(actual));
    lequal(1, (int)C_Table_size(t));

    // PUT key = value2
    lok(C_Table_put(t, &key, &value2));

    // Check key = value2
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value2), tostr(actual));
    lequal(1, (int)C_Table_size(t));

    teardown();
}

static void table_remove() {
    C_Userdata key, value;

    setup();

    C_Userdata_set_string(&key, "key");
    C_Userdata_set_string(&value, "value");

    lok(C_Table_add(t, &key, &value));

    lequal(C_Table_has(t, &key), true);
    lequal(1, (int)C_Table_size(t));

    lok(C_Table_remove(t, &key));

    lequal(C_Table_has(t, &key), false);
    lequal(0, (int)C_Table_size(t));

    teardown();
}

static kvpair* findpair(const char* key, kvpair* kvt, size_t sz) {
    if (!key) return NULL;

    for (int i = 0; i < sz; i++) {
        if (strcmp(key, kvt[i].key) == 0) {
            return &(kvt[i]);
        }
    }
    return NULL;
}

static void table_iterator() {
    C_Userdata key, value;
    kvpair expected[] = {
        { "alpha?",   "alpha!" },
        { "charlie?", "charlie!" },
        { "golf?",    "golf!" },
        { "kilo?",    "kilo!" },
        { "Saori",    "Hayami" },
        { NULL, NULL }
    };
    int expsz = 5;
    int count;
    C_Table_Iterator* i = NULL;

    setup();

    for (int i = 0; expected[i].key != NULL; i++) {
        // ADD key = value
        C_Userdata_set_string(&key, expected[i].key);
        C_Userdata_set_string(&value, expected[i].value);
        lok(C_Table_add(t, &key, &value));
    }

    C_Table_new_iterator(t, &i);
    lok(i != NULL);

    count = 0;
    while (C_Table_Iterator_has_next(i)) {
        kvpair* exprec;

        C_Table_Iterator_next(i);
        C_Userdata_clear(&key, false);
        C_Userdata_clear(&value, false);
        C_Table_Iterator_current_pair(i, &key, &value);

        // Won't be in order, so we'll have to do this the hard way ...
        exprec = findpair((const char *)key.ptr, expected, expsz); 

        lok(exprec != NULL);
        if (!exprec) break;

        lsequal(exprec->key,   (char*)key.ptr);
        lsequal(exprec->value, (char*)value.ptr);
        count++;

        if (count > expsz + 3) break;
    }
    C_Table_Iterator_free(&i);
    lequal(expsz, count);

    teardown();
}


int main (int argc, char* argv[]) {
    lrun("table_smoke", table_smoke);
    lrun("table_get", table_get);
    lrun("table_has", table_has);
    lrun("table_add", table_add);
    lrun("table_add_multiple", table_add_multiple);
    lrun("table_put", table_put);
    lrun("table_remove", table_remove);
    lrun("table_with_pointer_key", table_with_pointer_key);
    lrun("table_with_pointer_value", table_with_pointer_value);
    lrun("table_iterator", table_iterator);
    lresults();
    return lfails != 0;
}
