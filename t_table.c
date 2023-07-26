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
#include "minctest.h"
#include "ctable.h"

static C_Table* t = NULL;

static const char* tostr(C_Userdata ud) {
    static char* last_result;
    static char* result;

    free(last_result);

    last_result = result;

    result = calloc(50 + ud.len, sizeof(char));

    if (ud.len == 0 || ud.ptr == NULL) {
        sprintf(result, "{%d,%ld,%p},", ud.tag, ud.len, ud.ptr);
    } else {
        sprintf(result, "{%d,%ld,\"%s\"}", ud.tag, ud.len, (char*)ud.ptr);
    }
    return result;
}

static void setup() {
    t = NULL;
    C_Table_new(&t, 3);
    lok(t != NULL);
}

static void teardown() {
    C_Table_free(&t);
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

    // Try to add the key again
    lequal(C_Table_add(t, &key, &value2), false);

    // Check key = value in table
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value), tostr(actual));

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

    // PUT key = value
    lok(C_Table_put(t, &key, &value));

    // Check key = value
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value), tostr(actual));

    // PUT key = value2
    lok(C_Table_put(t, &key, &value2));

    // Check key = value2
    C_Userdata_clear(&actual, false);
    lok(C_Table_get(t, &key, &actual));
    lsequal(tostr(value2), tostr(actual));

    teardown();
}

static void table_remove() {
    C_Userdata key, value;

    setup();

    C_Userdata_set_string(&key, "key");
    C_Userdata_set_string(&value, "value");

    lok(C_Table_add(t, &key, &value));

    lequal(C_Table_has(t, &key), true);

    lok(C_Table_remove(t, &key));

    lequal(C_Table_has(t, &key), false);

    teardown();
}

static void table_smoke() {
    setup();

    teardown();
}

int main (int argc, char* argv[]) {
    lrun("table_smoke", table_smoke);
    lrun("table_get", table_get);
    lrun("table_has", table_has);
    lrun("table_add", table_add);
    lrun("table_put", table_put);
    lrun("table_remove", table_remove);
    lresults();
    return lfails != 0;
}
