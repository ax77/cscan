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
#include <setjmp.h>
#include <time.h>

#include "hashmap.h"
#include "core_unittests.h"
#include "core_mem.h"
#include "core_array.h"

#define __READ_RBP() __asm__ volatile("movq %%rbp, %0" : "=r"(__rbp))
#define __READ_RSP() __asm__ volatile("movq %%rsp, %0" : "=r"(__rsp))

intptr_t * __rbp;
intptr_t * __rsp;
intptr_t * __stackBegin;

static HashMap *HEAP = NULL;

static const size_t GC_KB = 1024;
static const size_t GC_MB = 1024 * 1024;
static const size_t LIMIT = 256; // (1024 * 1024) * 8; // when do we need run gc

static size_t ALLOCATED = 0;
static size_t TOTALLY_ALLOCATED = 0;
static size_t TOTALLY_DEALLOCATED = 0;
static size_t GC_INVOKED = 0;
static size_t MARK_MSEC = 0;
static size_t SWEEP_MSEC = 0;

#define CHECK_HARD_IS_EXISTS(ptr) cc_assert_true(HashMap_get(HEAP, ptr))

static void gcInit()
{
    __READ_RBP();
    __stackBegin = (intptr_t *) __rbp;
}

static void *gc_malloc(size_t size)
{
    assert(size);
    assert(size <= INT_MAX);

    void *ret = NULL;
    ret = malloc(size);
    if (ret == NULL) {
        ret = malloc(size);
        if (ret == NULL) {
            ret = malloc(size);
        }
    }

    assert(ret);
    return ret;
}

static void *gc_realloc(void *ptr, size_t newsize)
{
    assert(newsize);
    assert(newsize <= INT_MAX);

    void *ret = NULL;
    ret = realloc(ptr, newsize);
    if (ret == NULL) {
        ret = realloc(ptr, newsize);
        if (ret == NULL) {
            ret = realloc(ptr, newsize);
        }
    }

    assert(ret);
    return ret;
}

struct gc_memory {
    void *ptr;
    size_t size;
    int marked;
    char *location; // debug
};

static ArrayList *get_roots();
static ArrayList *get_pointers(struct gc_memory *mem);
static void mark();
static void sweep();
static void free_mem(struct gc_memory **mem);
static void heap_print();

static void run_gc(const char *_file, const char *_func, int _line)
{
    GC_INVOKED += 1;

    printf("\n----------------------------------------------------------------------\n");
    printf("\nBEFORE RUNNING GC from: %s: %s: %d\n", _file, _func, _line);
    heap_print();

    /// MARK

    clock_t before_mark = clock();
    printf("\nAFTER MARK\n");
    mark();
    heap_print();
    MARK_MSEC += (clock() - before_mark) * 1000 / CLOCKS_PER_SEC;

    /// SWEEP

    clock_t before_sweep = clock();
    printf("\nAFTER SWEEP\n");
    sweep();
    heap_print();
    SWEEP_MSEC += (clock() - before_sweep) * 1000 / CLOCKS_PER_SEC;
}

static char *create_position_info(const char *_file, const char *_func, int _line)
{
    char buffer[512] = { '\0' };
    sprintf(buffer, "%s:%s:%d", _file, _func, _line);
    return cc_strdup(buffer);
}

static void put_memory_header_into_the_heap(void *ptr, size_t size, const char *_file,
        const char *_func, int _line)
{

    assert(ptr);
    assert(size);

    struct gc_memory *mem = gc_malloc(sizeof(struct gc_memory));
    mem->ptr = ptr;
    mem->size = size;
    mem->marked = 0;
    mem->location = create_position_info(_file, _func, _line);
    HashMap_add(HEAP, ptr, mem);
}

#define alloc_mem(size) alloc_mem_internal(size, __FILE__, __func__, __LINE__)
static void *alloc_mem_internal(size_t size, const char *_file, const char *_func, int _line)
{

    void *ptr = gc_malloc(size);

    put_memory_header_into_the_heap(ptr, size, _file, _func, _line);

    /// XXX: not sure how to do this properly...
    if (ALLOCATED >= LIMIT) {
        run_gc(_file, _func, _line);
    }

    ALLOCATED += size;
    TOTALLY_ALLOCATED += size;
    return ptr;
}

#define realloc_mem(ptr, newsize) realloc_mem_internal(ptr, newsize, __FILE__, __func__, __LINE__)
static void *realloc_mem_internal(void *ptr, size_t newsize, const char *_file, const char *_func,
        int _line)
{
    assert(ptr);
    assert(newsize);

    struct gc_memory *mem = HashMap_remove(HEAP, ptr);
    if (!mem) {
        cc_fatal("pointer was not found in the heap: %p\n", ptr);
    }

    assert(mem->ptr);
    assert(mem->size);

    // create a new pointer, and copy all the bytes from the old one
    // it is possible to use realloc here
    // but - using malloc is much straightforward in my opinion.
    // dunno why.
    void *newptr = gc_malloc(newsize);
    memcpy(newptr, mem->ptr, mem->size);

    // manually destroy the old object
    // we've already deleted it from the heap
    // now we have to free the content
    free_mem(&mem);

    // create new object, and put it into the heap
    put_memory_header_into_the_heap(newptr, newsize, _file, _func, _line);

    /// XXX: not sure how to do this properly...
    if (ALLOCATED >= LIMIT) {
        run_gc(_file, _func, _line);
    }

    // re-calculate the size
    ALLOCATED += newsize;
    TOTALLY_ALLOCATED += newsize;
    return newptr;
}

static void free_mem(struct gc_memory **mem)
{
    assert(*mem);
    assert((*mem)->ptr);
    assert((*mem)->size);

    free((*mem)->ptr);
    (*mem)->ptr = NULL;

    free((*mem)->location);
    (*mem)->location = NULL;

    ALLOCATED -= (*mem)->size;
    TOTALLY_DEALLOCATED += (*mem)->size;
    (*mem)->size = 0;

    free(*mem);
    *mem = NULL;
}

static size_t ptr_hash(void *ptr)
{
    return (size_t) ptr;
}

static int ptr_eq(void *a, void *b)
{
    return a == b;
}

static ArrayList *get_roots()
{

    ArrayList *result = array_new(&array_dummy_free_fn);

    jmp_buf jb;
    setjmp(jb);

    __READ_RSP();
    uint8_t * rsp = (uint8_t *) __rsp;
    uint8_t * top = (uint8_t *) __stackBegin;

    assert(rsp);
    assert(top);

    while (rsp < top) {
        // auto address = (Traceable *) *(uintptr_t *) rsp;

        void * address = (void*) *(uintptr_t *) rsp;

        if (address) {
            struct gc_memory *mem = HashMap_get(HEAP, address);
            if (mem) {
                array_add(result, mem);
            }
        }

        rsp++;
    }

    return result;
}

static ArrayList *get_pointers(struct gc_memory *mem)
{
    ArrayList *result = array_new(&array_dummy_free_fn);

    uint8_t *p = mem->ptr;
    uint8_t *end = p + mem->size;

    assert(p);
    assert(end);

    while (p < end) {

        void * address = (void*) *(uintptr_t *) p;

        if (address) {
            struct gc_memory *mem = HashMap_get(HEAP, address);
            if (mem) {
                array_add(result, mem);
            }
        }

        p++;
    }

    return result;
}

static void mark()
{

    ArrayList *worklist = get_roots();

    while (!array_is_empty(worklist)) {
        struct gc_memory *mem = array_pop_back(worklist);
        if (!mem->marked) {
            mem->marked = 1;
            ArrayList *pointers = get_pointers(mem);
            for (size_t i = 0; i < pointers->size; i += 1) {
                struct gc_memory *ptr = (struct gc_memory *) array_get(pointers, i);
                array_add(worklist, ptr);
            }
            array_free(pointers);
        }
    }

    array_free(worklist);
}

static void sweep()
{

    ArrayList *to_remove = array_new(&array_dummy_free_fn);

    for (size_t i = 0; i < HEAP->capacity; i++) {

        Entry* e = HEAP->table[i];
        if (e == NULL) {
            continue;
        }

        for (; e; e = e->next) {
            struct gc_memory *mem = (struct gc_memory *) e->val;
            if (mem->marked) {
                mem->marked = 0;
            } else {
                array_add(to_remove, mem);
            }
        }
    }

    for (size_t i = 0; i < to_remove->size; i += 1) {
        struct gc_memory *mem = (struct gc_memory *) array_get(to_remove, i);
        void *ptr = mem->ptr;
        HashMap_remove(HEAP, ptr);
        free_mem(&mem);
    }

    array_free(to_remove);
}

static void heap_print()
{
    static char *delim =
            "|----------------|----------------|--|----------------|----------------------------------------------------------------|\n";

    printf(delim);
    printf("|%16s|%16s|%2s|%16s|%64s|\n", "pointer", "object", "M", "size", "location");
    printf(delim);

    for (size_t i = 0; i < HEAP->capacity; i++) {
        Entry* e = HEAP->table[i];
        if (e == NULL) {
            continue;
        }
        for (; e; e = e->next) {
            struct gc_memory *mem = (struct gc_memory *) e->val;
            printf("|%16lu|%16lu|%2s|%16lu|%-64s|\n", (size_t) e->key, (size_t) e->val,
                    (mem->marked ? "V" : " "), mem->size, mem->location);
            printf(delim);
        }
    }

    printf("TOTAL SIZE=%lu, capacity=%lu\n", HEAP->size, HEAP->capacity);
}

void * getstrmem()
{
    void *str1 = alloc_mem(1);
    void *str2 = alloc_mem(2);
    void *str3 = alloc_mem(3);
    void *str4 = alloc_mem(4);
    void *str5 = alloc_mem(5);
    void *str6 = alloc_mem(6);
    void *str7 = alloc_mem(7);

    CHECK_HARD_IS_EXISTS(str1);
    CHECK_HARD_IS_EXISTS(str2);
    CHECK_HARD_IS_EXISTS(str3);
    CHECK_HARD_IS_EXISTS(str4);
    CHECK_HARD_IS_EXISTS(str5);
    CHECK_HARD_IS_EXISTS(str6);
    CHECK_HARD_IS_EXISTS(str7);

    return str1;
}

char* anotherfn()
{
    char *str = alloc_mem(128);
    CHECK_HARD_IS_EXISTS(str);
    return str;
}

void dummy1()
{
    char *ptr = alloc_mem(12);
    CHECK_HARD_IS_EXISTS(ptr);
}

void dummy2()
{
    char *ptr = alloc_mem(1212);
    CHECK_HARD_IS_EXISTS(ptr);
}

void dummy4(void *ptr)
{
    ptr = alloc_mem(10);
}

void dummy3()
{
    char *ptr = alloc_mem(121212);
    CHECK_HARD_IS_EXISTS(ptr);
    dummy4(ptr);
}

void * runintime()
{
    int msec = 0, trigger = 1000 * 5; /* 1000 * sec */
    clock_t before = clock();

    void *ptr = NULL;

    do {
        /*
         * Do something to busy the CPU just here while you drink a coffee
         * Be sure this code will not take more than `trigger` ms
         */

        ptr = alloc_mem(32768);
        CHECK_HARD_IS_EXISTS(ptr);

        clock_t difference = clock() - before;
        msec = difference * 1000 / CLOCKS_PER_SEC;
    } while (msec < trigger);

    printf("Time taken %d seconds %d milliseconds\n", msec / 1000, msec % 1000);
    return ptr;
}

void test_array()
{
    ArrayList *arr = array_new(&free);
    array_add(arr, cc_strdup("1"));
    array_add(arr, cc_strdup("2"));
    array_add(arr, cc_strdup("3"));

    cc_assert_true(arr->size == 3);
    char *str = array_pop_back(arr);
    cc_assert_true(strcmp(str, "3") == 0);
    cc_assert_true(arr->size == 2);

    str = array_pop_back(arr);
    cc_assert_true(strcmp(str, "2") == 0);
    cc_assert_true(arr->size == 1);

    str = array_pop_back(arr);
    cc_assert_true(strcmp(str, "1") == 0);
    cc_assert_true(arr->size == 0);

    array_free(arr);
}

void empty()
{
    void *xxx = alloc_mem(3030);
    CHECK_HARD_IS_EXISTS(xxx);
}

void test_loop_another(void *ptr)
{
    CHECK_HARD_IS_EXISTS(ptr);
}

void test_loop()
{
    for (size_t i = 0; i < 8; i += 1) {
        void *ptr = alloc_mem(32768);
        CHECK_HARD_IS_EXISTS(ptr);
        test_loop_another(ptr);
    }
}

void print_stat()
{

    size_t total_time = MARK_MSEC + SWEEP_MSEC;

    printf("\n------- STAT ------- \n");

    printf("TOTALLY_ALLOCATED   %lu bytes, %lu Kb, %lu Mb\n", TOTALLY_ALLOCATED,
            TOTALLY_ALLOCATED / GC_KB, TOTALLY_ALLOCATED / GC_MB);

    printf("TOTALLY_DEALLOCATED %lu bytes, %lu Kb, %lu Mb\n", TOTALLY_DEALLOCATED,
            TOTALLY_DEALLOCATED / GC_KB, TOTALLY_DEALLOCATED / GC_MB);

    printf("GC_INVOKED %lu times\n", GC_INVOKED);
    printf("MARK       sec:%lu, msec:%lu\n", MARK_MSEC / 1000, MARK_MSEC % 1000);
    printf("SWEEP      sec:%lu, msec:%lu\n", SWEEP_MSEC / 1000, SWEEP_MSEC % 1000);
    printf("TOTAL      sec:%lu, msec:%lu\n", total_time / 1000, total_time % 1000);

    printf("\nHEAP NOW \n");
    heap_print();
}

int do_main(int argc, char **argv)
{
    HEAP = HashMap_new(ptr_hash, ptr_eq);
    ALLOCATED = 0;
    TOTALLY_ALLOCATED = 0;
    GC_INVOKED = 0;
    MARK_MSEC = 0;
    SWEEP_MSEC = 0;

    test_loop();

    char *str = alloc_mem(32);
    CHECK_HARD_IS_EXISTS(str);

    str = realloc_mem(str, 128);
    CHECK_HARD_IS_EXISTS(str);

    str = realloc_mem(str, 1024);
    str = realloc_mem(str, 10240);
    CHECK_HARD_IS_EXISTS(str);

    //void *xxxxx = runintime();
    //test_array();

//    anotherfn();
//    char*str = anotherfn();
//    void*ptr = getstrmem();
//    dummy1();
//    dummy2();
//    dummy3();
//    empty();
//
//    CHECK_HARD_IS_EXISTS(str);
//    CHECK_HARD_IS_EXISTS(ptr);

    print_stat();
    printf("\n:ok:\n");
    return 0;
}

int main(int argc, char **argv)
{
    gcInit();
    return do_main(argc, argv);
}

