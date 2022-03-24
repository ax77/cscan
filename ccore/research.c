#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>

static char* internal_strdup(char *str)
{
    assert(str);
    size_t newlen = strlen(str) + 1;
    char *newstr = (char*) calloc(1u, newlen);
    assert(newstr);
    strcpy(newstr, str);
    return newstr;
}

// Why do we use [size_t *alloc] instead of [size_t alloc]?
// Yes, we're able to use this function without the pointer
// which change in place an alloc variable, and invoke this
// in a form like: varalloc = realloc_array(&arr, arr_nr, varalloc);
// But: what if we'll forget (and we will) about the assignment?
// It will cause a silly behaviour.
// Doing so (dealing with a pointer to var that holds the space) -
// we guarantee that we won't forget to update the allocated
// space of the vec we update here.
//
size_t realloc_array1(void ***vec, size_t *nr, size_t *alloc)
{
    //static_assert((CHAR_BIT * sizeof(void*)) == 64, "64");

    assert(vec);
    assert(nr);
    assert(alloc);
    assert(*alloc < INT_MAX);

    // We cannot check safety that the *vec is NULL or it's not,
    // because there may be a real garbage which is not actually
    // a NULL value, or it may be an uninitialized stack-variable, etc.
    // So - our decision is to use alloc value as an indicator of memory
    // usage of this *vec.
    if (*alloc == 0) {
        *alloc = 2;
        *vec = calloc(1u, *alloc * sizeof(void*));
        assert(*vec);
        return *alloc;
    }

    // If we're here - *vec must not be NULL.
    // But: it may be a dangling pointer nevertheless.
    // So - there's no safety here.
    assert(*vec);

    if (((*nr) + 1) >= *alloc) {
        *alloc += 2;
        *alloc *= 2;
        *vec = realloc(*vec, *alloc * sizeof(void*));
        assert(*vec);
    }

    return *alloc;
}

int vec_append(void ***vec, size_t *nr, size_t *alloc, void *elem)
{
    assert(vec);
    assert(nr);
    assert(alloc);

    realloc_array1(vec, nr, alloc);
    (*vec)[*nr] = elem;
    *nr += 1;

    return 1;
}

static void test_0()
{
    char **strings;
    size_t strings_nr = 0, strings_alloc = 0;

    for (int i = 0; i < 8; i++) {
        char buf[8] = { '\0' };
        sprintf(buf, "%d", i);
        vec_append((void***) &strings, &strings_nr, &strings_alloc,
                internal_strdup(buf));
    }

    for (int i = 0; i < strings_nr; i++) {
        printf("%s\n", strings[i]);
    }
}

struct args {
    char **strings;
    size_t strings_nr;
    size_t strings_alloc;
};

struct args collect_args()
{
    struct args var = { };

    for (int i = 0; i < 8; i++) {
        char buf[8] = { '\0' };
        sprintf(buf, "%d", i);
        vec_append((void***) &var.strings, &var.strings_nr, &var.strings_alloc,
                internal_strdup(buf));
    }

    return var;
}

void test_realloc_in_place()
{

    struct args var = collect_args();
    for (int i = 0; i < var.strings_nr; i++) {
        //printf("%s\n", var.strings[i]);
    }

}

