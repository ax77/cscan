#include "core_gc.h"

/// FUNCS
typedef size_t (*gcmap_hash_fn)(void *key);
typedef int (*gcmap_equal_fn)(void *a, void *b);

/// HASHMAP
struct gc_map_ent {
    void *key;
    void *val;
    struct gc_map_ent *next;
};

struct gc_map {
    gcmap_hash_fn hash;
    gcmap_equal_fn equal;
    size_t size;
    size_t capacity;
    struct gc_map_ent **table;
    size_t threshold;
};
static struct gc_map* gc_map_new(gcmap_hash_fn hash, gcmap_equal_fn equal);
static void* gc_map_get(struct gc_map *self, void *key);
static void* gc_map_put(struct gc_map *self, void *key, void *value);
static void* gc_map_remove(struct gc_map *self, void *key);
static size_t gc_map_hash_ptr(void *ptr);
static int gc_map_equal_ptr(void *a, void *b);
static void* gc_map_entry_new(void *key, void *val, struct gc_map_ent *next);
static struct gc_map_ent** gc_map_empty_table(size_t capacity);
static size_t gc_map_index(struct gc_map *self, void *key, size_t capacity);
static const size_t GC_MAP_DEFAULT_CAPACITY = 11;
static const float GC_MAP_LOAD_FACTOR = 0.75;

/// ARRAY
struct gc_array {
    void **data;
    size_t size, alloc;
};
static struct gc_array* gc_array_new();
static void gc_array_add(struct gc_array *array, void *ptr);
static void* gc_array_get(struct gc_array *array, size_t at_index);
static void* gc_array_pop_back(struct gc_array *array);
static void gc_array_free(struct gc_array *array);
static int gc_array_is_empty(struct gc_array *array);

/// GC
struct gc_ptr {
    void *ptr;
    size_t size;
    int marked;
    char *location; // debug
};

static struct gc_array* gc_get_roots(struct core_gc *gc);
static struct gc_array* gc_get_pointers(struct core_gc *gc, struct gc_ptr *mem);
static void gc_mark(struct core_gc *gc);
static void gc_sweep(struct core_gc *gc);
static void gc_free_mem(struct core_gc *gc, struct gc_ptr **mem);
static void gc_print_heap(struct core_gc *gc);

/// GENERAL

#define gc_fatal(fmt, ...) gc_internal_fatal(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
static void gc_internal_fatal(const char *_file, int _line, const char *_func,
        const char *fmt, ...) {
    va_list vl;
    char buffer[512];

    va_start(vl, fmt);
    vsprintf(buffer, fmt, vl);
    va_end(vl);

    fprintf( stderr, "FATAL: (%s:[%5d]:%s()) : %s\n", _file, _line, _func,
            buffer);
    exit(128);
}

static void* aux_unmanaged_malloc(size_t size) {
    assert(size);
    assert(size <= INT_MAX);

    void *ret = NULL;
    ret = calloc(1u, size);
    if (ret == NULL) {
        ret = calloc(1u, size);
        if (ret == NULL) {
            ret = calloc(1u, size);
        }
    }

    assert(ret);
    return ret;
}

static void* aux_unmanaged_realloc(void *ptr, size_t newsize) {
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

static char* aux_unmanaged_strdup(char *str) {
    assert(str);
    size_t newlen = strlen(str) + 1;
    char *newstr = (char*) aux_unmanaged_malloc(newlen);
    strcpy(newstr, str);
    return newstr;
}

static size_t aux_max(size_t a, size_t b) {
    if (a > b) {
        return a;
    }
    return b;
}

static size_t aux_min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

/// HASHMAP

static void* gc_map_entry_new(void *key, void *val, struct gc_map_ent *next) {
    struct gc_map_ent *entry = (struct gc_map_ent*) aux_unmanaged_malloc(
            sizeof(struct gc_map_ent));
    entry->key = key;
    entry->val = val;
    entry->next = next;
    return entry;
}

static struct gc_map_ent** gc_map_empty_table(size_t capacity) {
    assert(capacity);

    struct gc_map_ent **table = (struct gc_map_ent**) aux_unmanaged_malloc(
            sizeof(struct gc_map_ent*) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        table[i] = NULL;
    }
    return table;
}

static struct gc_map* gc_map_new(gcmap_hash_fn hash, gcmap_equal_fn equal) {
    assert(hash);
    assert(equal);

    struct gc_map *hashmap = (struct gc_map*) aux_unmanaged_malloc(
            sizeof(struct gc_map));
    hashmap->hash = hash;
    hashmap->equal = equal;
    hashmap->size = 0;
    hashmap->capacity = GC_MAP_DEFAULT_CAPACITY;
    hashmap->table = gc_map_empty_table(hashmap->capacity);
    hashmap->threshold = hashmap->capacity * GC_MAP_LOAD_FACTOR;
    return hashmap;
}

static size_t gc_map_index(struct gc_map *self, void *key, size_t capacity) {
    return self->hash(key) % capacity;
}

static void* gc_map_get(struct gc_map *self, void *key) {
    assert(self);
    assert(key);

    size_t index = gc_map_index(self, key, self->capacity);
    for (struct gc_map_ent *e = self->table[index]; e; e = e->next) {
        if (self->equal(e->key, key)) {
            return e->val;
        }
    }
    return NULL;
}

static void* gc_map_put(struct gc_map *self, void *key, void *val) {
    assert(self);
    assert(key);

    size_t index = gc_map_index(self, key, self->capacity);

    // overwrite
    for (struct gc_map_ent *e = self->table[index]; e; e = e->next) {
        if (self->equal(key, e->key)) {
            void *oldval = e->val;
            e->val = val;
            return oldval;
        }
    }

    // resize
    if (self->size > self->threshold) {

        // calculate new capacity and build new empty table
        size_t new_capacity = self->capacity * 2 + 1;
        struct gc_map_ent **new_table = gc_map_empty_table(new_capacity);

        // rehash
        for (size_t i = 0; i < self->capacity; i++) {
            struct gc_map_ent *next = NULL;
            for (struct gc_map_ent *e = self->table[i]; e; e = next) {
                next = e->next;
                size_t index = gc_map_index(self, e->key, new_capacity);
                e->next = new_table[index];
                new_table[index] = e;
            }
        }

        // set new table and free the old one
        free(self->table);
        self->table = new_table;
        self->capacity = new_capacity;
        self->threshold = new_capacity * GC_MAP_LOAD_FACTOR;

        // get the index according to new capacity
        index = gc_map_index(self, key, self->capacity);
    }

    // add
    struct gc_map_ent *new_entry = gc_map_entry_new(key, val,
            self->table[index]);
    self->table[index] = new_entry;
    self->size++;
    return NULL;
}

static void* gc_map_remove(struct gc_map *self, void *key) {
    assert(self);
    assert(key);

    size_t index = gc_map_index(self, key, self->capacity);
    struct gc_map_ent *e = self->table[index];
    if (e == NULL) {
        return NULL;
    }

    struct gc_map_ent *prev = NULL;
    struct gc_map_ent *next = NULL;
    for (; e; prev = e, e = next) {
        next = e->next;
        if (self->equal(key, e->key)) {
            void *val = e->val;
            if (prev == NULL) {
                self->table[index] = next;
            } else {
                prev->next = next;
            }
            self->size--;
            free(e);
            return val;
        }
    }
    return NULL;
}

static size_t gc_map_hash_ptr(void *ptr) {
    return (size_t) ptr;
}

static int gc_map_equal_ptr(void *a, void *b) {
    return a == b;
}

/// ARRAY

static struct gc_array* gc_array_new() {
    struct gc_array *rv = aux_unmanaged_malloc(sizeof(struct gc_array));
    rv->alloc = 2;
    rv->size = 0;
    rv->data = aux_unmanaged_malloc(sizeof(void*) * rv->alloc);
    for (size_t i = 0; i < rv->alloc; i++) {
        rv->data[i] = NULL;
    }

    return rv;
}

static void gc_array_add(struct gc_array *array, void *ptr) {
    assert(array);

    if (array->size >= array->alloc) {
        array->alloc *= 2;
        array->data = aux_unmanaged_realloc(array->data,
                sizeof(void*) * array->alloc);
    }
    array->data[array->size] = ptr;
    array->size++;
}

static void* gc_array_get(struct gc_array *array, size_t at_index) {
    assert(at_index < array->size);
    return array->data[at_index];
}

static void* gc_array_pop_back(struct gc_array *array) {
    assert(array);
    if (array->size == 0) {
        return NULL;
    }

    void *elem = array->data[array->size - 1];
    array->size -= 1;

    return elem;
}

static void gc_array_free(struct gc_array *array) {
    assert(array);
    free(array->data);
    free(array);
}

static int gc_array_is_empty(struct gc_array *array) {
    assert(array);
    return array->size == 0;
}

/// GC

void gc_init(struct core_gc *gc, void *bottom) {

    assert(gc);
    assert(bottom);

    gc->bottom = bottom;
    gc->minptr = UINTPTR_MAX;
    gc->maxptr = 0;

    gc->bytes_allocated = 0;
    gc->limit = 256;
    gc->heap = gc_map_new(gc_map_hash_ptr, gc_map_equal_ptr);

    gc->times_invoked = 0;
    gc->mark_msec = 0;
    gc->sweep_msec = 0;
    gc->totally_allocated = 0;
    gc->totally_deallocated = 0;
}

static void assert_gc(struct core_gc *gc) {
    assert(gc);
    assert(gc->heap);
}

static void run_gc(struct core_gc *gc, const char *_file, const char *_func,
        int _line) {
    assert_gc(gc);

    gc->times_invoked += 1;

    if (PRINT_GC_INVOKED_STAT) {
        printf(
                "\n----------------------------------------------------------------------\n");
        printf("\nBEFORE RUNNING GC from: %s: %s: %d\n", _file, _func, _line);
        gc_print_heap(gc);
    }

    /// MARK

    clock_t before_mark = clock();
    gc_mark(gc);
    gc->mark_msec += (clock() - before_mark) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER MARK\n");
        gc_print_heap(gc);
    }

    /// SWEEP

    clock_t before_sweep = clock();
    gc_sweep(gc);
    gc->sweep_msec += (clock() - before_sweep) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER SWEEP\n");
        gc_print_heap(gc);
    }
}

static char* create_position_info(const char *_file, const char *_func,
        int _line) {
    char buffer[512] = { '\0' };
    sprintf(buffer, "%s:%s:%d", _file, _func, _line);
    return aux_unmanaged_strdup(buffer);
}

static void put_memory_header_into_the_heap(struct core_gc *gc, void *ptr,
        size_t size, const char *_file, const char *_func, int _line) {

    assert_gc(gc);
    assert(ptr);
    assert(size);

    struct gc_ptr *mem = aux_unmanaged_malloc(sizeof(struct gc_ptr));
    mem->ptr = ptr;
    mem->size = size;
    mem->marked = 0;
    mem->location = create_position_info(_file, _func, _line);

    size_t iptr = (size_t) ptr;
    gc->maxptr = aux_max(gc->maxptr, iptr);
    gc->minptr = aux_min(gc->minptr, iptr);

    // do not allow to overwrite the pair
    // if this pair is already presented by this key...
    // is it possible?
    void *overwritten = gc_map_put(gc->heap, ptr, mem);
    assert(overwritten == NULL);

    /// XXX: not sure how to do this properly...
    if (gc->bytes_allocated >= gc->limit) {
        run_gc(gc, _file, _func, _line);
    }

    gc->bytes_allocated += size;
    gc->totally_allocated += size;
}

void* gc_alloc_mem_internal(struct core_gc *gc, size_t size, const char *_file,
        const char *_func, int _line) {
    assert_gc(gc);
    void *ptr = aux_unmanaged_malloc(size);
    put_memory_header_into_the_heap(gc, ptr, size, _file, _func, _line);
    return ptr;
}

void* gc_realloc_mem_internal(struct core_gc *gc, void *ptr, size_t newsize,
        const char *_file, const char *_func, int _line) {
    assert_gc(gc);
    assert(ptr);
    assert(newsize);

    struct gc_ptr *mem = gc_map_remove(gc->heap, ptr);
    if (!mem) {
        gc_fatal("pointer was not found in the heap: %p\n", ptr);
    }

    assert(mem->ptr);
    assert(mem->size);

    // create a new pointer, and copy all the bytes from the old one
    // it is possible to use realloc here
    // but - using malloc is much straightforward in my opinion.
    // dunno why.
    void *newptr = aux_unmanaged_malloc(newsize);
    memcpy(newptr, mem->ptr, mem->size);

    // manually destroy the old object
    // we've already deleted it from the heap
    // now we have to free the content
    gc_free_mem(gc, &mem);

    // create new object, and put it into the heap
    put_memory_header_into_the_heap(gc, newptr, newsize, _file, _func, _line);
    return newptr;
}

char* gc_strdup_internal(struct core_gc *gc, char *str, const char *_file,
        const char *_func, int _line) {
    assert_gc(gc);
    assert(str);
    size_t newlen = strlen(str) + 1;
    char *newstr = (char*) gc_alloc_mem_internal(gc, newlen, _file, _func,
            _line);
    strcpy(newstr, str);
    return newstr;
}

static void gc_free_mem(struct core_gc *gc, struct gc_ptr **mem) {
    assert_gc(gc);
    assert(*mem);
    assert((*mem)->ptr);
    assert((*mem)->size);

    gc->bytes_allocated -= (*mem)->size;
    gc->totally_deallocated += (*mem)->size;

    free((*mem)->ptr);
    (*mem)->ptr = NULL;

    free((*mem)->location);
    (*mem)->location = NULL;

    (*mem)->size = 0;
    (*mem)->marked = 0;

    free(*mem);
    *mem = NULL;
}

static struct gc_array* gc_get_roots(struct core_gc *gc) {
    assert_gc(gc);

    void *__rsp = NULL;

    jmp_buf jb;
    setjmp(jb);

    uint8_t *rsp = (uint8_t*) &__rsp;
    uint8_t *top = (uint8_t*) gc->bottom;

    assert(rsp);
    assert(top);

    // TODO: rsp<top, rsp>top

    struct gc_array *result = gc_array_new();
    for (; rsp < top; rsp++) {

        void *address = (void*) *(uintptr_t*) rsp;
        //printf("%lu, %lu\n", address, *((void**)rsp));

        size_t iptr = (size_t) address;
        if (iptr == 0) {
            continue;
        }
        if (iptr < gc->minptr || iptr > gc->maxptr) {
            continue;
        }

        struct gc_ptr *mem = gc_map_get(gc->heap, address);
        if (mem) {
            gc_array_add(result, mem);
        }

    }

    return result;
}

static struct gc_array* gc_get_pointers(struct core_gc *gc, struct gc_ptr *mem) {
    assert_gc(gc);
    struct gc_array *result = gc_array_new();

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
        if (iptr < gc->minptr || iptr > gc->maxptr) {
            continue;
        }

        struct gc_ptr *mem = gc_map_get(gc->heap, address);
        if (mem) {
            gc_array_add(result, mem);
        }

    }

    return result;
}

static void gc_mark(struct core_gc *gc) {
    assert_gc(gc);

    struct gc_array *worklist = gc_get_roots(gc);

    while (!gc_array_is_empty(worklist)) {
        struct gc_ptr *mem = gc_array_pop_back(worklist);
        if (!mem->marked) {
            mem->marked = 1;
            struct gc_array *pointers = gc_get_pointers(gc, mem);
            for (size_t i = 0; i < pointers->size; i += 1) {
                struct gc_ptr *ptr = (struct gc_ptr*) gc_array_get(pointers, i);
                gc_array_add(worklist, ptr);
            }
            gc_array_free(pointers);
        }
    }

    gc_array_free(worklist);
}

static void gc_sweep(struct core_gc *gc) {
    assert_gc(gc);

    struct gc_array *to_remove = gc_array_new();

    for (size_t i = 0; i < gc->heap->capacity; i++) {

        struct gc_map_ent *e = gc->heap->table[i];
        if (e == NULL) {
            continue;
        }

        for (; e; e = e->next) {
            struct gc_ptr *mem = (struct gc_ptr*) e->val;
            if (mem->marked) {
                mem->marked = 0;
            } else {
                gc_array_add(to_remove, mem);
            }
        }
    }

    for (size_t i = 0; i < to_remove->size; i += 1) {
        struct gc_ptr *mem = (struct gc_ptr*) gc_array_get(to_remove, i);
        void *ptr = mem->ptr;
        gc_map_remove(gc->heap, ptr);
        gc_free_mem(gc, &mem);
    }

    gc_array_free(to_remove);
}

static void gc_print_heap(struct core_gc *gc) {
    assert_gc(gc);

    static char *delim =
            "|----------------"
                    "|----------------"
                    "|--"
                    "|----------------"
                    "|----------------------------------------------------------------|\n";

    printf("%s", delim);
    printf("|%16s|%16s|%2s|%16s|%64s|\n", "pointer", "object", "M", "size",
            "location");
    printf("%s", delim);

    for (size_t i = 0; i < gc->heap->capacity; i++) {
        struct gc_map_ent *e = gc->heap->table[i];
        if (e == NULL) {
            continue;
        }
        for (; e; e = e->next) {
            struct gc_ptr *mem = (struct gc_ptr*) e->val;
            printf("|%16lu|%16lu|%2s|%16lu|%-64s|\n", (size_t) e->key,
                    (size_t) e->val, (mem->marked ? "V" : " "), mem->size,
                    mem->location);
            printf("%s", delim);
        }
    }

    printf("TOTAL SIZE=%lu, capacity=%lu\n", gc->heap->size,
            gc->heap->capacity);
}

void gc_forced_free_internal(struct core_gc *gc, void *ptr, const char *_file,
        const char *_func, int _line) {
    struct gc_ptr *mem = gc_map_remove(gc->heap, ptr);
    assert(mem);
    gc_free_mem(gc, &mem);
}

void gc_print_stat(struct core_gc *gc) {
    assert_gc(gc);

    size_t total_time = gc->mark_msec + gc->sweep_msec;

    printf("\n------- STAT ------- \n");

    printf("TOTALLY_ALLOCATED   %lu bytes, %lu Kb, %lu Mb\n",
            gc->totally_allocated, gc->totally_allocated / GC_KB,
            gc->totally_allocated / GC_MB);

    printf("TOTALLY_DEALLOCATED %lu bytes, %lu Kb, %lu Mb\n",
            gc->totally_deallocated, gc->totally_deallocated / GC_KB,
            gc->totally_deallocated / GC_MB);

    printf("GC_INVOKED %lu times\n", gc->times_invoked);
    printf("MARK       sec:%lu, msec:%lu\n", gc->mark_msec / 1000,
            gc->mark_msec % 1000);
    printf("SWEEP      sec:%lu, msec:%lu\n", gc->sweep_msec / 1000,
            gc->sweep_msec % 1000);
    printf("TOTAL      sec:%lu, msec:%lu\n", total_time / 1000,
            total_time % 1000);

    printf("\nHEAP NOW \n");
    gc_print_heap(gc);
}

