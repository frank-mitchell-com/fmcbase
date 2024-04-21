#include <stdio.h>
#include "minctest.h"

static void test1() {
    lok('a' == 'a');
}

static void test2() {
    lequal(5, 5);
    lfequal(5.5, 5.5);
    lsequal("abc", "abc");
}

int main (int argc, char* argv[]) {
    lrun("test1", test1);
    lrun("test2", test2);
    lresults();
    return lfails != 0;
}
