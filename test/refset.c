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
#include "crefset.h"

static C_Ref_Set* t = NULL;

static const char* EXPECT[] = {
    "alpha",
    "bravo",
    "charlie",
    "delta",
    "echo",
    "foxtrot",
    "golf",
    "hotel",
    "india",
    "juliet",
    "kilo",
    "lima",
    "mike",
    "november",
    "oscar",
    "papa",
    "quebec",
    "romeo",
    "sierra",
    "tango",
    "uniform",
    "victor",
    "whiskey",
    "x-ray",
    "yankee",
    "zebra"
};

const int EXPECTSZ = sizeof(EXPECT)/sizeof(const char*);

static void setup() {
    t = NULL;
    C_Ref_Set_new(&t, 3);
    lok(t != NULL);
}

static void teardown() {
    C_Ref_Set_free(&t);
}

static void refset_smoke() {
    setup();

    teardown();
}

static void refset_add() {
    setup();

    for (int i = 0; i < EXPECTSZ; i++) {
        // ADD key = value
        lok(C_Ref_Set_add(t, EXPECT[i]));

        // Check key = value in table
        lok(C_Ref_Set_has(t, EXPECT[i]));
    }

    lequal(EXPECTSZ, (int)C_Ref_Set_size(t));

    for (int i = 0; i < EXPECTSZ; i++) {
        // Check key = value in table AGAIN
        lok(C_Ref_Set_has(t, EXPECT[i]));
    }

    teardown();
}

static void refset_remove() {
    const char* data = "example";

    setup();

    lok(C_Ref_Set_add(t, data));

    lequal(true, C_Ref_Set_has(t, data));
    lequal(1, (int)C_Ref_Set_size(t));

    lok(C_Ref_Set_remove(t, data));

    lequal(false, C_Ref_Set_has(t, data));
    lequal(0, (int)C_Ref_Set_size(t));

    teardown();
}

static void refset_iterator() {
    C_Ref_Set_Iterator* i = NULL;
    int total = 0;
    int count[EXPECTSZ];

    setup();

    for (int i = 0; i < EXPECTSZ; i++) {
        // ADD key = value
        lok(C_Ref_Set_add(t, EXPECT[i]));
        count[i] = 0;
    }

    C_Ref_Set_new_iterator(t, &i);
    lok(i != NULL);

    while (C_Ref_Set_Iterator_has_next(i)) {
        const char* actual;
        bool found = false;

        C_Ref_Set_Iterator_next(i);
        actual = (const char*)C_Ref_Set_Iterator_current(i);

        lok(actual != NULL);

        // Won't be in order, so we'll have to do this the hard way ...
        for (int j = 0; j < EXPECTSZ; j++) {
            if (strcmp(EXPECT[j], actual) == 0) {
                count[j]++;
                break;
            }
        }

        total++;
        if (total > EXPECTSZ + 6) break;
    }
    C_Ref_Set_Iterator_free(&i);
    lequal(EXPECTSZ, total);
    for (int i = 0; i < EXPECTSZ; i++) {
        lequal(1, count[i]);
    }

    teardown();
}


int main (int argc, char* argv[]) {
    lrun("refset_smoke", refset_smoke);
    lrun("refset_add", refset_add);
    lrun("refset_remove", refset_remove);
    lrun("refset_iterator", refset_iterator);
    lresults();
    return lfails != 0;
}
