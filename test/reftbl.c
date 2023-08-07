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
#include "reftable.h"

static C_Ref_Table* t = NULL;

typedef struct _kvpair {
    const char* key; 
    const char* value;
} kvpair;

static void setup() {
    t = NULL;
    C_Ref_Table_new(&t, 3);
    lok(t != NULL);
}

static void teardown() {
    C_Ref_Table_free(&t);
}

static void reftbl_smoke() {
    setup();

    teardown();
}

static void reftbl_put() {
    const void* value;
    const void* oldvalue = "splunge";

    setup();

    // Check key not in table
    lequal(C_Ref_Table_has(t, "key"), false);

    // PUT key = value
    lok(C_Ref_Table_put(t, "key", "value", &oldvalue));
    lok(oldvalue == NULL);

    // Check key = value
    lsequal("value", (const char *)C_Ref_Table_get(t, "key"));

    // PUT key = value2
    lok(C_Ref_Table_put(t, "key", "value2", &oldvalue));
    lsequal("value", (const char*)oldvalue);

    // Check key = value2
    lsequal("value2", (const char*)C_Ref_Table_get(t, "key"));

    teardown();
}

static void reftbl_put_multiple() {
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
        lok(C_Ref_Table_put(t, key, value, NULL));

        // Check key = value in table
        lsequal(value, (const char*)C_Ref_Table_get(t, key));
    }

    for (int i = 0; expected[i].key != NULL; i++) {
        const char* key = expected[i].key;
        const char* value = expected[i].value;

        // Check key = value in table AGAIN
        lsequal(value, (const char*)C_Ref_Table_get(t, key));
    }

    teardown();
}

static void reftbl_remove() {
    const void* oldvalue = "-not value-";

    setup();

    lok(C_Ref_Table_put(t, "key", "value", NULL));

    lequal(C_Ref_Table_has(t, "key"), true);

    lok(C_Ref_Table_remove(t, "key", &oldvalue));
    lsequal("value", (const char*)oldvalue);

    lequal(C_Ref_Table_has(t, "key"), false);

    teardown();
}

int main (int argc, char* argv[]) {
    lrun("reftbl_smoke", reftbl_smoke);
    lrun("reftbl_put", reftbl_put);
    lrun("reftbl_put_multiple", reftbl_put_multiple);
    lrun("reftbl_remove", reftbl_remove);
    lresults();
    return lfails != 0;
}
