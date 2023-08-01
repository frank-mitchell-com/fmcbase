
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
#include "crefcnt.h"

#define STRBUFSIZ   20

static void refcnt_count() {
    const char* tobj = strdup("this is only a test");
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
}

int main (int argc, char* argv[]) {
    lrun("refcnt_count", refcnt_count);
    lresults();
    return lfails != 0;
}
