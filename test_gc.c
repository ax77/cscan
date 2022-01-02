#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core_gc.h"

struct core_gc GC;

int do_main(int argc, char **argv) {
    char *str = gc_malloc(&GC, 1024);
    str = gc_realloc(&GC, str, 2048);

    for (int i = 0; i < 8; i += 1) {
        str = gc_malloc(&GC, 32768);
    }

    gc_print_stat(&GC);

    gc_forced_free(&GC, str);
    gc_print_stat(&GC);


    printf("\n:ok:\n");
    return 0;
}

int main(int argc, char **argv) {
    gc_init(&GC, &argc);
    return do_main(argc, argv);
}

