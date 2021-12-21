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
#include "core_list.h"
#include "core_array.h"

#define __READ_RBP() __asm__ volatile("movq %%rbp, %0" : "=r"(__rbp))
#define __READ_RSP() __asm__ volatile("movq %%rsp, %0" : "=r"(__rsp))

intptr_t * __rbp;
intptr_t * __rsp;
intptr_t * __stackBegin;

static HashMap *heap = NULL;
static size_t ALLOCATED = 0;
static const size_t LIMIT = 132; // (1024 * 1024) * 8; // when do we need run gc
static size_t TOTALLY_ALLOCATED = 0;

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

struct gc_memory {
    void *ptr;
    size_t size;
    int marked;
    char *location; // debug
};

static LinkedList *getRoots();
static LinkedList *getPointers(struct gc_memory *mem);
static void mark();
static void sweep();
static void free_mem(struct gc_memory **mem);
static void heap_print();

static void run_gc()
{
    printf("\n----------------------------------------------------------------------\n");
    printf("\nBEFORE RUNNING GC\n");
    heap_print();

    printf("\nAFTER MARK\n");
    mark();
    heap_print();

    printf("\nAFTER SWEEP\n");
    sweep();
    heap_print();
}

#define alloc_mem(size) alloc_mem_internal(size, __FILE__, __func__, __LINE__)
static void *alloc_mem_internal(size_t size, const char *_file, const char *_func, int _line)
{

    void *ptr = gc_malloc(size);

    struct gc_memory *mem = gc_malloc(sizeof(struct gc_memory));
    mem->ptr = ptr;
    mem->size = size;
    mem->marked = 0;

    char buffer[512] = { '\0' };
    sprintf(buffer, "[%s:%s:%d]", _file, _func, _line);
    mem->location = cc_strdup(buffer);

    HashMap_add(heap, ptr, mem);

    /// XXX: I'm not sure how to do this properly...
    if (ALLOCATED >= LIMIT) {
        run_gc();
    }
    ALLOCATED += size;
    TOTALLY_ALLOCATED += size;
    return ptr;
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

static LinkedList *getRoots()
{
    /// roots finder

    LinkedList *roots = list_new(ptr_eq);

    jmp_buf jb;
    setjmp(jb);

    __READ_RSP();
    uint8_t * rsp = (uint8_t *) __rsp;
    uint8_t * top = (uint8_t *) __stackBegin;

    while (rsp < top) {
        // auto address = (Traceable *) *(uintptr_t *) rsp;

        void * address = (void*) *(uintptr_t *) rsp;

        if (!address) {
            rsp++;
            continue;
        }

        struct gc_memory *mem = HashMap_get(heap, address);
        if (mem) {
            list_push_back(roots, mem);
        }

        rsp++;
    }

    return roots;
}

static LinkedList *getPointers(struct gc_memory *mem)
{
    LinkedList *result = list_new(ptr_eq);

    uint8_t *p = mem->ptr;
    uint8_t *end = p + mem->size;

    while (p < end) {

        void * address = (void*) *(uintptr_t *) p;
        if (!address) {
            p++;
            continue;
        }

        struct gc_memory *mem = HashMap_get(heap, address);
        if (mem) {
            list_push_back(result, mem);
        }

        p++;
    }

    return result;
}

static void mark()
{

    LinkedList *worklist = getRoots();
    while (!list_is_empty(worklist)) {
        struct gc_memory *mem = list_pop_back(worklist);
        if (!mem->marked) {
            mem->marked = 1;
            LinkedList *pointers = getPointers(mem);
            for (ListNode * node = pointers->first; node; node = node->next) {
                struct gc_memory *ptr = (struct gc_memory *) node->item;
                list_push_back(worklist, ptr);
            }
        }
    }
}

static void sweep()
{

    ArrayList *to_remove = array_new(&array_dummy_free_fn);

    for (size_t i = 0; i < heap->capacity; i++) {

        Entry* e = heap->table[i];
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
        HashMap_remove(heap, ptr);
        free_mem(&mem);
    }

    array_free(to_remove);
}

static void heap_print()
{
    printf("heap: size=%lu, capacity=%lu\n", heap->size, heap->capacity);

    for (size_t i = 0; i < heap->capacity; i++) {
        Entry* e = heap->table[i];
        if (e == NULL) {
            continue;
        }
        for (; e; e = e->next) {
            struct gc_memory *mem = (struct gc_memory *) e->val;
            printf("%lu:%lu:%s:%-4lu:%s\n", (size_t) e->key, (size_t) e->val,
                    (mem->marked ? "V" : " "), mem->size, mem->location);
        }
    }
}

void * getstrmem()
{
    void *str1 = alloc_mem(8);
    void *str2 = alloc_mem(8);
    void *str3 = alloc_mem(8);
    void *str4 = alloc_mem(8);
    void *str5 = alloc_mem(8);
    void *str6 = alloc_mem(8);
    void *str7 = alloc_mem(8);

    return str1;
}

char* anotherfn()
{
    char *str = alloc_mem(128);
    return str;
}

void runintime()
{
    int msec = 0, trigger = 1000 * 5; /* 1000 * sec */
    clock_t before = clock();

    do {
        /*
         * Do something to busy the CPU just here while you drink a coffee
         * Be sure this code will not take more than `trigger` ms
         */

        void *ptr = alloc_mem(32768);

        clock_t difference = clock() - before;
        msec = difference * 1000 / CLOCKS_PER_SEC;
    } while (msec < trigger);

    printf("Time taken %d seconds %d milliseconds\n", msec / 1000, msec % 1000);
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

int do_main(int argc, char **argv)
{
    heap = HashMap_new(ptr_hash, ptr_eq);
    ALLOCATED = 0;
    //runintime();

    //test_array();

//    anotherfn();
//    char*str = anotherfn();
//    void*ptr = getstrmem();

    printf("\nTOTALLY_ALLOCATED %lu Mb\n", TOTALLY_ALLOCATED / 1024 / 1024);
    printf("\n:ok:\n");
    return 0;
}

int main(int argc, char **argv)
{
    gcInit();
    return do_main(argc, argv);
}

