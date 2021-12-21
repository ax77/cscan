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
static const size_t LIMIT = 32; // (1024 * 1024) * 8; // when do we need run gc

static size_t ALLOCATED = 0;
static size_t TOTALLY_ALLOCATED = 0;
static size_t TOTALLY_DEALLOCATED = 0;
static size_t GC_INVOKED = 0;
static size_t MARK_MSEC = 0;
static size_t SWEEP_MSEC = 0;

#define PRINT_GC_INVOKED_STAT (0)

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

    if (PRINT_GC_INVOKED_STAT) {
        printf("\n----------------------------------------------------------------------\n");
        printf("\nBEFORE RUNNING GC from: %s: %s: %d\n", _file, _func, _line);
        heap_print();
    }

    /// MARK

    clock_t before_mark = clock();
    mark();
    MARK_MSEC += (clock() - before_mark) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER MARK\n");
        heap_print();
    }

    /// SWEEP

    clock_t before_sweep = clock();
    sweep();
    SWEEP_MSEC += (clock() - before_sweep) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER SWEEP\n");
        heap_print();
    }
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

#define gc_strdup(str) gc_strdup_internal(str, __FILE__, __func__, __LINE__)
static char *gc_strdup_internal(char *str, const char *_file, const char *_func, int _line)
{
    assert(str);
    size_t newlen = strlen(str) + 1;
    char * newstr = (char*) alloc_mem_internal(newlen, _file, _func, _line);
    strcpy(newstr, str);
    return newstr;
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

//// linked list for tests

struct gc_list_node {
    void *item;
    struct gc_list_node *prev;
    struct gc_list_node *next;
};

struct gc_linked_list {
    struct gc_list_node *first, *last;
    size_t size;
};

static struct gc_linked_list * gc_list_new()
{
    struct gc_linked_list *list = alloc_mem(sizeof(struct gc_linked_list));
    assert(list && "list malloc");

    list->first = list->last = NULL;
    list->size = 0;

    return list;
}

static struct gc_list_node * gc_list_node_new(struct gc_list_node *prev, void *e,
        struct gc_list_node *next)
{
    struct gc_list_node *node = alloc_mem(sizeof(struct gc_list_node));
    assert(node && "node malloc");
    node->prev = prev;
    node->item = e;
    node->next = next;
    return node;
}

static void gc_list_push_back(struct gc_linked_list *list, void *e)
{
    struct gc_list_node *l = list->last;
    struct gc_list_node *n = gc_list_node_new(l, e, NULL);
    list->last = n;
    if (l == NULL) {
        list->first = n;
    } else {
        l->next = n;
    }
    list->size++;
}

static void * gc_list_pop_back(struct gc_linked_list *list)
{
    struct gc_list_node *l = list->last;
    assert(l);

    void *elem = l->item;

    struct gc_list_node *prev = l->prev;

    l->item = NULL;
    l->prev = NULL; // help GC

    list->last = prev;
    if (prev == NULL) {
        list->first = NULL;
    } else {
        prev->next = NULL;
    }
    list->size--;

    return elem;
}

static int gc_list_empty(struct gc_linked_list *list)
{
    return list->size == 0;
}

void dothelist1(struct gc_linked_list *list)
{
    while (!gc_list_empty(list)) {
        char *str = gc_list_pop_back(list);
        CHECK_HARD_IS_EXISTS(str);
        printf("%s\n", str);
    }
}

void dothelist2(struct gc_linked_list *list)
{
    for (int i = 0; i < 8; i += 1) {
        char buf[32];
        sprintf(buf, "%d", i);
        gc_list_push_back(list, gc_strdup(buf));
    }
    dothelist1(list);
}

void test_list()
{
    struct gc_linked_list *list = gc_list_new();
    for (int i = 0; i < 32; i += 1) {
        char buf[32];
        sprintf(buf, "%d", i);
        gc_list_push_back(list, gc_strdup(buf));
    }
    dothelist1(list);
    dothelist2(list);
}

int do_main(int argc, char **argv)
{
    HEAP = HashMap_new(ptr_hash, ptr_eq);
    ALLOCATED = 0;
    TOTALLY_ALLOCATED = 0;
    GC_INVOKED = 0;
    MARK_MSEC = 0;
    SWEEP_MSEC = 0;

    test_list();
    char *str = alloc_mem(128);
    str = alloc_mem(128);

    //run_gc(__FILE__, __func__, __LINE__);

    print_stat();
    printf("\n:ok:\n");
    return 0;
}

int main(int argc, char **argv)
{
    gcInit();
    return do_main(argc, argv);
}

