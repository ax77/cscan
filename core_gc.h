/*
 * core_gc.h
 *
 *  Created on: 22 дек. 2021 г.
 *      Author: Alexey
 */

#ifndef CORE_GC_H_
#define CORE_GC_H_

/// we have to keep this GC files independent
/// from any other source code functions.
/// I mean - if we need an array, we must create
/// our own array in core_gc.c file, and use it
/// properly.
/// because - if we need to use our GC with other
/// libraries we need to keep straightforward implementation.

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>

#if defined __amd64__ \
||  defined __amd64 \
||  defined __x86_64__ \
||  defined __x86_64 \
||  defined _M_X64 \
||  defined _M_AMD64

#define __READ_RBP() __asm__ volatile("movq %%rbp, %0" : "=r"(__rbp))
#define __READ_RSP() __asm__ volatile("movq %%rsp, %0" : "=r"(__rsp))

#else
#error("unsupported CPU")
#endif

struct gc_map;

struct core_gc {

    // the bottom address, we will
    // remember it in the 'main' function
    void *bottom;

    // we will ignore 'pointers' which are not
    // satisfy this filter at the mark stage.
    size_t minptr;
    size_t maxptr;

    // bytes allocated at the current state
    // it will be changed at the sweep phase
    // it will be changed by 'realloc'
    size_t bytes_allocated;

    // we will run GC when bytes_allocated >= limit
    size_t limit;

    // of core_gc_ptr
    // where the key is an actual void*
    // and the value is an core_gc_ptr*
    struct gc_map *heap;

    // statistics
    size_t times_invoked;
    size_t mark_msec;
    size_t sweep_msec;
    size_t totally_allocated;
    size_t totally_deallocated;
};

void gc_init(struct core_gc *gc, void *bottom);
void gc_print_stat();

#define gc_malloc(size) gc_alloc_mem_internal(size, __FILE__, __func__, __LINE__)
void* gc_alloc_mem_internal(size_t size, const char *_file, const char *_func, int _line);

#define gc_realloc(ptr, newsize) gc_realloc_mem_internal(ptr, newsize, __FILE__, __func__, __LINE__)
void* gc_realloc_mem_internal(void *ptr, size_t newsize, const char *, const char *, int);

#define gc_strdup(str) gc_strdup_internal(str, __FILE__, __func__, __LINE__)
char* gc_strdup_internal(char *str, const char *_file, const char *_func, int _line);

/// UTIL

static const size_t GC_KB = 1024;
static const size_t GC_MB = 1024 * 1024;

#define PRINT_GC_INVOKED_STAT (0)

#endif /* CORE_GC_H_ */
