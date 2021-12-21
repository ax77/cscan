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
#include <dirent.h>

#include "hashmap.h"
#include "core_unittests.h"
#include "core_mem.h"
#include "core_array.h"
#include "core_strutil.h"

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

struct core_gc_ptr {
    void *ptr;
    size_t size;
    int marked;
    char *location; // debug
};

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
    HashMap *heap;

    // statistics
    size_t times_invoked;
    size_t mark_msec;
    size_t sweep_msec;
    size_t totally_allocated;
    size_t totally_deallocated;
};

struct core_gc GC;

static const size_t GC_KB = 1024;
static const size_t GC_MB = 1024 * 1024;
static const size_t LIMIT = 256; // (1024 * 1024) * 8; // when do we need run gc

#define PRINT_GC_INVOKED_STAT (0)

#define CHECK_HARD_IS_EXISTS(ptr) cc_assert_true(HashMap_get(GC.heap, ptr))

static size_t ptr_hash(void *ptr) {
    return (size_t) ptr;
}

static int ptr_eq(void *a, void *b) {
    return a == b;
}

static void gc_init(struct core_gc *gc, void *bottom) {

    assert(gc);
    assert(bottom);

    gc->bottom = bottom;
    gc->minptr = UINTPTR_MAX;
    gc->maxptr = 0;

    gc->bytes_allocated = 0;
    gc->limit = 256;
    gc->heap = HashMap_new(ptr_hash, ptr_eq);

    gc->times_invoked = 0;
    gc->mark_msec = 0;
    gc->sweep_msec = 0;
    gc->totally_allocated = 0;
    gc->totally_deallocated = 0;
}

static void* gc_malloc(size_t size) {
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

static void* gc_realloc(void *ptr, size_t newsize) {
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

static ArrayList* get_roots();
static ArrayList* get_pointers(struct core_gc_ptr *mem);
static void mark();
static void sweep();
static void free_mem(struct core_gc_ptr **mem);
static void heap_print();

static void run_gc(const char *_file, const char *_func, int _line) {
    GC.times_invoked += 1;

    if (PRINT_GC_INVOKED_STAT) {
        printf(
                "\n----------------------------------------------------------------------\n");
        printf("\nBEFORE RUNNING GC from: %s: %s: %d\n", _file, _func, _line);
        heap_print();
    }

    /// MARK

    clock_t before_mark = clock();
    mark();
    GC.mark_msec += (clock() - before_mark) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER MARK\n");
        heap_print();
    }

    /// SWEEP

    clock_t before_sweep = clock();
    sweep();
    GC.sweep_msec += (clock() - before_sweep) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER SWEEP\n");
        heap_print();
    }
}

static char* create_position_info(const char *_file, const char *_func,
        int _line) {
    char buffer[512] = { '\0' };
    sprintf(buffer, "%s:%s:%d", _file, _func, _line);
    return cc_strdup(buffer);
}

static size_t max(size_t a, size_t b) {
    if (a > b) {
        return a;
    }
    return b;
}

static size_t min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

static void put_memory_header_into_the_heap(void *ptr, size_t size,
        const char *_file, const char *_func, int _line) {

    assert(ptr);
    assert(size);

    struct core_gc_ptr *mem = gc_malloc(sizeof(struct core_gc_ptr));
    mem->ptr = ptr;
    mem->size = size;
    mem->marked = 0;
    mem->location = create_position_info(_file, _func, _line);

    size_t iptr = (size_t) ptr;
    GC.maxptr = max(GC.maxptr, iptr);
    GC.minptr = min(GC.minptr, iptr);

    HashMap_add(GC.heap, ptr, mem);
}

#define alloc_mem(size) alloc_mem_internal(size, __FILE__, __func__, __LINE__)
static void* alloc_mem_internal(size_t size, const char *_file,
        const char *_func, int _line) {

    void *ptr = gc_malloc(size);

    put_memory_header_into_the_heap(ptr, size, _file, _func, _line);

    /// XXX: not sure how to do this properly...
    if (GC.bytes_allocated >= GC.limit) {
        run_gc(_file, _func, _line);
    }

    GC.bytes_allocated += size;
    GC.totally_allocated += size;
    return ptr;
}

#define realloc_mem(ptr, newsize) realloc_mem_internal(ptr, newsize, __FILE__, __func__, __LINE__)
static void* realloc_mem_internal(void *ptr, size_t newsize, const char *_file,
        const char *_func, int _line) {
    assert(ptr);
    assert(newsize);

    struct core_gc_ptr *mem = HashMap_remove(GC.heap, ptr);
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
    if (GC.bytes_allocated >= GC.limit) {
        run_gc(_file, _func, _line);
    }

    // re-calculate the size
    GC.bytes_allocated += newsize;
    GC.totally_allocated += newsize;
    return newptr;
}

#define gc_strdup(str) gc_strdup_internal(str, __FILE__, __func__, __LINE__)
static char* gc_strdup_internal(char *str, const char *_file, const char *_func,
        int _line) {
    assert(str);
    size_t newlen = strlen(str) + 1;
    char *newstr = (char*) alloc_mem_internal(newlen, _file, _func, _line);
    strcpy(newstr, str);
    return newstr;
}

static void free_mem(struct core_gc_ptr **mem) {
    assert(*mem);
    assert((*mem)->ptr);
    assert((*mem)->size);

    free((*mem)->ptr);
    (*mem)->ptr = NULL;

    free((*mem)->location);
    (*mem)->location = NULL;

    GC.bytes_allocated -= (*mem)->size;
    GC.totally_deallocated += (*mem)->size;
    (*mem)->size = 0;

    free(*mem);
    *mem = NULL;
}

static ArrayList* get_roots() {

    void *__rsp = NULL;

    jmp_buf jb;
    setjmp(jb);

    uint8_t *rsp = (uint8_t*) &__rsp;
    uint8_t *top = (uint8_t*) GC.bottom;

    assert(rsp);
    assert(top);

    // TODO: rsp<top, rsp>top

    ArrayList *result = array_new(&array_dummy_free_fn);
    for (; rsp < top; rsp++) {

        void *address = (void*) *(uintptr_t*) rsp;
        //printf("%lu, %lu\n", address, *((void**)rsp));

        size_t iptr = (size_t) address;
        if (iptr == 0) {
            continue;
        }
        if (iptr < GC.minptr || iptr > GC.maxptr) {
            continue;
        }

        struct core_gc_ptr *mem = HashMap_get(GC.heap, address);
        if (mem) {
            array_add(result, mem);
        }

    }

    return result;
}

static ArrayList* get_pointers(struct core_gc_ptr *mem) {
    ArrayList *result = array_new(&array_dummy_free_fn);

    uint8_t *p = (uint8_t*) mem->ptr;
    uint8_t *end = (uint8_t*) (p + mem->size);

    assert(p);
    assert(end);

    for (; p < end; p++) {

        void *address = (void*) *(uintptr_t*) p;

        size_t iptr = (size_t) address;
        if (iptr == 0) {
            continue;
        }
        if (iptr < GC.minptr || iptr > GC.maxptr) {
            continue;
        }

        struct core_gc_ptr *mem = HashMap_get(GC.heap, address);
        if (mem) {
            array_add(result, mem);
        }

    }

    return result;
}

static void mark() {

    ArrayList *worklist = get_roots();

    while (!array_is_empty(worklist)) {
        struct core_gc_ptr *mem = array_pop_back(worklist);
        if (!mem->marked) {
            mem->marked = 1;
            ArrayList *pointers = get_pointers(mem);
            for (size_t i = 0; i < pointers->size; i += 1) {
                struct core_gc_ptr *ptr = (struct core_gc_ptr*) array_get(
                        pointers, i);
                array_add(worklist, ptr);
            }
            array_free(pointers);
        }
    }

    array_free(worklist);
}

static void sweep() {

    ArrayList *to_remove = array_new(&array_dummy_free_fn);

    for (size_t i = 0; i < GC.heap->capacity; i++) {

        Entry *e = GC.heap->table[i];
        if (e == NULL) {
            continue;
        }

        for (; e; e = e->next) {
            struct core_gc_ptr *mem = (struct core_gc_ptr*) e->val;
            if (mem->marked) {
                mem->marked = 0;
            } else {
                array_add(to_remove, mem);
            }
        }
    }

    for (size_t i = 0; i < to_remove->size; i += 1) {
        struct core_gc_ptr *mem = (struct core_gc_ptr*) array_get(to_remove, i);
        void *ptr = mem->ptr;
        HashMap_remove(GC.heap, ptr);
        free_mem(&mem);
    }

    array_free(to_remove);
}

static void heap_print() {
    static char *delim =
            "|----------------|----------------|--|----------------|----------------------------------------------------------------|\n";

    printf("%s", delim);
    printf("|%16s|%16s|%2s|%16s|%64s|\n", "pointer", "object", "M", "size",
            "location");
    printf("%s", delim);

    for (size_t i = 0; i < GC.heap->capacity; i++) {
        Entry *e = GC.heap->table[i];
        if (e == NULL) {
            continue;
        }
        for (; e; e = e->next) {
            struct core_gc_ptr *mem = (struct core_gc_ptr*) e->val;
            printf("|%16lu|%16lu|%2s|%16lu|%-64s|\n", (size_t) e->key,
                    (size_t) e->val, (mem->marked ? "V" : " "), mem->size,
                    mem->location);
            printf("%s", delim);
        }
    }

    printf("TOTAL SIZE=%lu, capacity=%lu\n", GC.heap->size, GC.heap->capacity);
}

void* runintime() {
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

void test_loop_another(void *ptr) {
    CHECK_HARD_IS_EXISTS(ptr);
}

void test_loop() {
    for (size_t i = 0; i < 8; i += 1) {
        void *ptr = alloc_mem(32768);
        CHECK_HARD_IS_EXISTS(ptr);
        test_loop_another(ptr);
    }
}

void print_stat() {

    size_t total_time = GC.mark_msec + GC.sweep_msec;

    printf("\n------- STAT ------- \n");

    printf("TOTALLY_ALLOCATED   %lu bytes, %lu Kb, %lu Mb\n",
            GC.totally_allocated, GC.totally_allocated / GC_KB,
            GC.totally_allocated / GC_MB);

    printf("TOTALLY_DEALLOCATED %lu bytes, %lu Kb, %lu Mb\n",
            GC.totally_deallocated, GC.totally_deallocated / GC_KB,
            GC.totally_deallocated / GC_MB);

    printf("GC_INVOKED %lu times\n", GC.times_invoked);
    printf("MARK       sec:%lu, msec:%lu\n", GC.mark_msec / 1000,
            GC.mark_msec % 1000);
    printf("SWEEP      sec:%lu, msec:%lu\n", GC.sweep_msec / 1000,
            GC.sweep_msec % 1000);
    printf("TOTAL      sec:%lu, msec:%lu\n", total_time / 1000,
            total_time % 1000);

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

static struct gc_linked_list* gc_list_new() {
    struct gc_linked_list *list = alloc_mem(sizeof(struct gc_linked_list));
    assert(list && "list malloc");

    list->first = list->last = NULL;
    list->size = 0;

    return list;
}

static struct gc_list_node* gc_list_node_new(struct gc_list_node *prev, void *e,
        struct gc_list_node *next) {
    struct gc_list_node *node = alloc_mem(sizeof(struct gc_list_node));
    assert(node && "node malloc");
    node->prev = prev;
    node->item = e;
    node->next = next;
    return node;
}

static void gc_list_push_back(struct gc_linked_list *list, void *e) {
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

static void* gc_list_pop_back(struct gc_linked_list *list) {
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

static int gc_list_empty(struct gc_linked_list *list) {
    return list->size == 0;
}

void dothelist1(struct gc_linked_list *list) {
    while (!gc_list_empty(list)) {
        char *str = gc_list_pop_back(list);
        CHECK_HARD_IS_EXISTS(str);
        printf("%s\n", str);
    }
}

void dothelist2(struct gc_linked_list *list) {
    for (int i = 0; i < 8; i += 1) {
        char buf[32];
        sprintf(buf, "%d", i);
        gc_list_push_back(list, gc_strdup(buf));
    }
    dothelist1(list);
}

void test_list() {
    struct gc_linked_list *list = gc_list_new();
    for (int i = 0; i < 32; i += 1) {
        char buf[32];
        sprintf(buf, "%d", i);
        gc_list_push_back(list, gc_strdup(buf));
    }
    dothelist1(list);
    dothelist2(list);
}

static char* gc_readfile(const char *filename) {
    FILE *fp = NULL;
    size_t n, sz;

    char *data = NULL;
    fp = fopen(filename, "rb");
    assert(fp && "file does not exists.");

    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    rewind(fp);

    data = alloc_mem(sz + 1);

    data[sz] = '\0';
    n = fread(data, 1, sz, fp);
    if (n != sz) {
        goto fail;
    }

    fclose(fp);
    return data;

    fail: if (fp) {
        fclose(fp);
    }

    assert(0);
    return NULL;
}

static void test_with_files() {
    DIR *d = NULL;
    struct dirent *dir;
    char *path = ".....";
    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {

            if (!strends(dir->d_name, "gctestext")) {
                continue;
            }

            char buf[2048] = { '\0' };
            sprintf(buf, "%s/%s", path, dir->d_name);

            char *content = gc_readfile(buf);
        }
        closedir(d);
    }
}

int do_main(int argc, char **argv) {

    test_list();
    char *str = alloc_mem(1024);
    str = realloc_mem(str, 2048);

    print_stat();
    printf("\n:ok:\n");
    return 0;
}

int main(int argc, char **argv) {
    gc_init(&GC, &argc);
    return do_main(argc, argv);
}

