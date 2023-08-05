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
#include "minctest.h"
#include "cstrtbl.h"

static C_String_Table* t = NULL;

typedef struct _kvpair {
    const char* key; 
    const char* value;
} kvpair;

static void setup() {
    t = NULL;
    C_String_Table_new(&t, 3);
    lok(t != NULL);
}

static void teardown() {
    C_String_Table_free(&t);
}

static void table_smoke() {
    setup();

    teardown();
}

static void table_add() {
    const void* value;

    setup();

    // Check key not in table
    lequal(C_String_Table_has(t, 3, "key"), false);
    lequal(0, (int)C_String_Table_size(t));

    // PUT key = value
    lok(C_String_Table_add(t, 3, "key", "value"));

    // Check key = value
    lsequal("value", (const char *)C_String_Table_get(t, 3, "key"));

    // Attempt to add key = value2
    lequal(false, C_String_Table_add(t, 3, "key", "value2"));

    // Check key = value2
    lsequal("value", (const char*)C_String_Table_get(t, 3, "key"));
    lequal(1, (int)C_String_Table_size(t));

    teardown();
}

static void table_add_multiple() {
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
        { NULL, NULL }
    };

    setup();

    for (int i = 0; expected[i].key != NULL; i++) {
        const char* key = expected[i].key;
        const char* value = expected[i].value;

        // ADD key = value
        lok(C_String_Table_add(t, strlen(key), key, value));

        // Check key = value in table
        lsequal(value, (const char*)C_String_Table_get(t, strlen(key), key));
    }

    lequal(13, (int)C_String_Table_size(t));

    for (int i = 0; expected[i].key != NULL; i++) {
        const char* key = expected[i].key;
        const char* value = expected[i].value;

        // Check key = value in table AGAIN
        lsequal(value, (const char*)C_String_Table_get(t, strlen(key), key));
    }

    teardown();
}

static void table_remove() {
    const void* oldvalue = "-not value-";

    setup();

    lok(C_String_Table_add(t, 3, "key", "value"));

    lequal(C_String_Table_has(t, 3, "key"), true);
    lequal(1, (int)C_String_Table_size(t));

    lok(C_String_Table_remove(t, 3, "key", &oldvalue));
    lsequal("value", (const char*)oldvalue);

    lequal(C_String_Table_has(t, 3, "key"), false);
    lequal(0, (int)C_String_Table_size(t));

    teardown();
}

int main (int argc, char* argv[]) {
    lrun("table_smoke", table_smoke);
    lrun("table_add", table_add);
    lrun("table_add_multiple", table_add_multiple);
    lrun("table_remove", table_remove);
    lresults();
    return lfails != 0;
}
