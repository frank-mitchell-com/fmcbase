
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
#include "refcount.h"

#define STRBUFSIZ   20

static void refcnt_count() {
    char* tobj = strdup("this is only a test");
    uint32_t result = 0;

    C_Ref_Count_list(tobj);
    lequal(1, C_Ref_Count_refcount(tobj));
    lok(C_Ref_Count_is_listed(tobj));

    result = C_Ref_Count_increment(tobj);
    lequal(2, result);
    result = C_Ref_Count_increment(tobj);
    lequal(3, result);
    result = C_Ref_Count_increment(tobj);
    lequal(4, result);

    lequal(4, C_Ref_Count_refcount(tobj));

    result = C_Ref_Count_decrement(tobj);
    lequal(3, result);
    result = C_Ref_Count_decrement(tobj);
    lequal(2, result);

    lequal(2, C_Ref_Count_refcount(tobj));

    result = C_Ref_Count_decrement(tobj);
    lequal(1, result);

    lequal(1, C_Ref_Count_refcount(tobj));

    result = C_Ref_Count_decrement(tobj);
    lequal(0, result);

    lequal(0, C_Ref_Count_refcount(tobj));

    result = C_Ref_Count_decrement(tobj);
    lequal(0, result);

    lequal(0, C_Ref_Count_refcount(tobj));

    C_Ref_Count_delist(tobj);
    lequal(1, C_Ref_Count_refcount(tobj));
    lequal(false, C_Ref_Count_is_listed(tobj));

    free(tobj);
}

static void refcnt_retain() {
    char* tobj  = strdup("this is only a test");
    const void* tobj2 = NULL;
    const void* result;

    C_Ref_Count_list(tobj);
    lequal(1, C_Ref_Count_refcount(tobj));
    lok(C_Ref_Count_is_listed(tobj));

    result = C_Any_retain(tobj);
    lok(tobj == result);
    lequal(2, C_Ref_Count_refcount(tobj));

    C_Any_set(&tobj2, tobj);
    lok(tobj == tobj2);
    lequal(3, C_Ref_Count_refcount(tobj));

    C_Any_set(&tobj2, NULL);
    lok(NULL == tobj2);
    lequal(2, C_Ref_Count_refcount(tobj));

    C_Any_release(&result);
    lok(NULL == result);
    lequal(1, C_Ref_Count_refcount(tobj));

    C_Ref_Count_delist(tobj);
    free(tobj);
}

static const void* _onzero_expect = NULL;
static bool        _onzero_called = false;

static void test_onzero(void* p) {
    _onzero_called = true;
    lok(_onzero_expect == p);
}

static void refcnt_onzero() {
    char* tobj  = strdup("this is only a test");

    C_Ref_Count_list(tobj);
    lequal(true, C_Ref_Count_is_listed(tobj));

    C_Ref_Count_on_zero(tobj, &test_onzero);

    _onzero_called = false;
    _onzero_expect = tobj;
    C_Ref_Count_decrement(tobj);

    lok(_onzero_called);
    lequal(false, C_Ref_Count_is_listed(tobj));
    lequal(1, C_Ref_Count_refcount(tobj));
}

int main (int argc, char* argv[]) {
    lrun("refcnt_count", refcnt_count);
    lrun("refcnt_retain", refcnt_retain);
    lrun("refcnt_onzero", refcnt_onzero);
    lresults();
    return lfails != 0;
}
