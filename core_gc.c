#include "core_gc.h"

struct core_gc GC;

/// FUNCS
typedef size_t (*gcmap_hash_fn)(void* key);
typedef int (*gcmap_equal_fn)(void* a, void* b);

/// HASHMAP
struct gc_map_ent {
    void* key;
    void* val;
    struct gc_map_ent* next;
};

struct gc_map {
    gcmap_hash_fn hash;
    gcmap_equal_fn equal;
    size_t size;
    size_t capacity;
    struct gc_map_ent** table;
    size_t threshold;
};
static struct gc_map* map_new(gcmap_hash_fn hash, gcmap_equal_fn equal);
static void* map_get(struct gc_map* self, void* key);
static void* map_put(struct gc_map* self, void* key, void* value);
static void* map_remove(struct gc_map* self, void* key);
static size_t map_hash_ptr(void *ptr);
static int map_equal_ptr(void *a, void *b);
static void* map_entry_new(void* key, void* val, struct gc_map_ent* next);
static struct gc_map_ent **map_empty_table(size_t capacity);
static size_t map_index(struct gc_map* self, void* key, size_t capacity);
static const size_t MAP_DEFAULT_CAPACITY = 11;
static const float MAP_LOAD_FACTOR = 0.75;

/// ARRAY
struct gc_array {
    void ** data;
    size_t size, alloc;
};
static struct gc_array * array_new();
static void array_add(struct gc_array *array, void *ptr);
static void *array_get(struct gc_array *array, size_t at_index);
static void *array_pop_back(struct gc_array *array);
static void array_free(struct gc_array *array);
static int array_is_empty(struct gc_array *array);

/// GC
struct gc_ptr {
    void *ptr;
    size_t size;
    int marked;
    char *location; // debug
};

static struct gc_array* gc_get_roots();
static struct gc_array* gc_get_pointers(struct gc_ptr *mem);
static void gc_mark();
static void gc_sweep();
static void gc_free_mem(struct gc_ptr **mem);
static void gc_print_heap();

/// GENERAL

#define cc_fatal(fmt, ...) internal_fatal(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
static void internal_fatal(const char *_file, int _line, const char *_func, const char *fmt, ...)
{
    va_list vl;
    char buffer[512];

    va_start(vl, fmt);
    vsprintf(buffer, fmt, vl);
    va_end(vl);

    fprintf( stderr, "FATAL: (%s:[%5d]:%s()) : %s\n", _file, _line, _func, buffer);
    exit(128);
}

static void* unmanaged_malloc(size_t size)
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

static void *unmanaged_realloc(void *ptr, size_t newsize)
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

static char *unmanaged_strdup(char *str)
{
    assert(str);
    size_t newlen = strlen(str) + 1;
    char * newstr = (char*) unmanaged_malloc(newlen);
    strcpy(newstr, str);
    return newstr;
}

static size_t max(size_t a, size_t b)
{
    if (a > b) {
        return a;
    }
    return b;
}

static size_t min(size_t a, size_t b)
{
    if (a < b) {
        return a;
    }
    return b;
}

/// HASHMAP

static void* map_entry_new(void* key, void* val, struct gc_map_ent* next)
{
    struct gc_map_ent* entry = (struct gc_map_ent*) unmanaged_malloc(sizeof(struct gc_map_ent));
    entry->key = key;
    entry->val = val;
    entry->next = next;
    return entry;
}

static struct gc_map_ent **map_empty_table(size_t capacity)
{
    assert(capacity);

    struct gc_map_ent **table = (struct gc_map_ent**) unmanaged_malloc(
            sizeof(struct gc_map_ent*) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        table[i] = NULL;
    }
    return table;
}

static struct gc_map* map_new(gcmap_hash_fn hash, gcmap_equal_fn equal)
{
    assert(hash);
    assert(equal);

    struct gc_map* hashmap = (struct gc_map*) unmanaged_malloc(sizeof(struct gc_map));
    hashmap->hash = hash;
    hashmap->equal = equal;
    hashmap->size = 0;
    hashmap->capacity = MAP_DEFAULT_CAPACITY;
    hashmap->table = map_empty_table(hashmap->capacity);
    hashmap->threshold = hashmap->capacity * MAP_LOAD_FACTOR;
    return hashmap;
}

static size_t map_index(struct gc_map* self, void* key, size_t capacity)
{
    return self->hash(key) % capacity;
}

static void* map_get(struct gc_map* self, void* key)
{
    assert(self);
    assert(key);

    size_t index = map_index(self, key, self->capacity);
    for (struct gc_map_ent* e = self->table[index]; e; e = e->next) {
        if (self->equal(e->key, key)) {
            return e->val;
        }
    }
    return NULL;
}

static void* map_put(struct gc_map* self, void* key, void* val)
{
    assert(self);
    assert(key);

    size_t index = map_index(self, key, self->capacity);

    // overwrite
    for (struct gc_map_ent* e = self->table[index]; e; e = e->next) {
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
        struct gc_map_ent** new_table = map_empty_table(new_capacity);

        // rehash
        for (size_t i = 0; i < self->capacity; i++) {
            struct gc_map_ent* next = NULL;
            for (struct gc_map_ent* e = self->table[i]; e; e = next) {
                next = e->next;
                size_t index = map_index(self, e->key, new_capacity);
                e->next = new_table[index];
                new_table[index] = e;
            }
        }

        // set new table and free the old one
        free(self->table);
        self->table = new_table;
        self->capacity = new_capacity;
        self->threshold = new_capacity * MAP_LOAD_FACTOR;

        // get the index according to new capacity
        index = map_index(self, key, self->capacity);
    }

    // add
    struct gc_map_ent* new_entry = map_entry_new(key, val, self->table[index]);
    self->table[index] = new_entry;
    self->size++;
    return NULL;
}

static void* map_remove(struct gc_map* self, void* key)
{
    assert(self);
    assert(key);

    size_t index = map_index(self, key, self->capacity);
    struct gc_map_ent* e = self->table[index];
    if (e == NULL) {
        return NULL;
    }

    struct gc_map_ent* prev = NULL;
    struct gc_map_ent* next = NULL;
    for (; e; prev = e, e = next) {
        next = e->next;
        if (self->equal(key, e->key)) {
            void* val = e->val;
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

static size_t map_hash_ptr(void *ptr)
{
    return (size_t) ptr;
}

static int map_equal_ptr(void *a, void *b)
{
    return a == b;
}

/// ARRAY

static struct gc_array * array_new()
{
    struct gc_array *rv = unmanaged_malloc(sizeof(struct gc_array));
    rv->alloc = 2;
    rv->size = 0;
    rv->data = unmanaged_malloc(sizeof(void*) * rv->alloc);
    for (size_t i = 0; i < rv->alloc; i++) {
        rv->data[i] = NULL;
    }

    return rv;
}

static void array_add(struct gc_array *array, void *ptr)
{
    assert(array);

    if (array->size >= array->alloc) {
        array->alloc *= 2;
        array->data = unmanaged_realloc(array->data, sizeof(void*) * array->alloc);
    }
    array->data[array->size] = ptr;
    array->size++;
}

static void *array_get(struct gc_array *array, size_t at_index)
{
    assert(at_index < array->size);
    return array->data[at_index];
}

static void * array_pop_back(struct gc_array *array)
{
    assert(array);
    if (array->size == 0) {
        return NULL;
    }

    void *elem = array->data[array->size - 1];
    array->size -= 1;

    return elem;
}

static void array_free(struct gc_array *array)
{
    assert(array);
    free(array->data);
    free(array);
}

static int array_is_empty(struct gc_array *array)
{
    assert(array);
    return array->size == 0;
}

/// GC

void gc_init(struct core_gc *gc, void *bottom)
{

    assert(gc);
    assert(bottom);

    gc->bottom = bottom;
    gc->minptr = UINTPTR_MAX;
    gc->maxptr = 0;

    gc->bytes_allocated = 0;
    gc->limit = 256;
    gc->heap = map_new(map_hash_ptr, map_equal_ptr);

    gc->times_invoked = 0;
    gc->mark_msec = 0;
    gc->sweep_msec = 0;
    gc->totally_allocated = 0;
    gc->totally_deallocated = 0;
}

static void run_gc(const char *_file, const char *_func, int _line)
{
    GC.times_invoked += 1;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\n----------------------------------------------------------------------\n");
        printf("\nBEFORE RUNNING GC from: %s: %s: %d\n", _file, _func, _line);
        gc_print_heap();
    }

    /// MARK

    clock_t before_mark = clock();
    gc_mark();
    GC.mark_msec += (clock() - before_mark) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER MARK\n");
        gc_print_heap();
    }

    /// SWEEP

    clock_t before_sweep = clock();
    gc_sweep();
    GC.sweep_msec += (clock() - before_sweep) * 1000 / CLOCKS_PER_SEC;

    if (PRINT_GC_INVOKED_STAT) {
        printf("\nAFTER SWEEP\n");
        gc_print_heap();
    }
}

static char* create_position_info(const char *_file, const char *_func, int _line)
{
    char buffer[512] = { '\0' };
    sprintf(buffer, "%s:%s:%d", _file, _func, _line);
    return unmanaged_strdup(buffer);
}

static void put_memory_header_into_the_heap(void *ptr, size_t size, const char *_file,
        const char *_func, int _line)
{

    assert(ptr);
    assert(size);

    struct gc_ptr *mem = unmanaged_malloc(sizeof(struct gc_ptr));
    mem->ptr = ptr;
    mem->size = size;
    mem->marked = 0;
    mem->location = create_position_info(_file, _func, _line);

    size_t iptr = (size_t) ptr;
    GC.maxptr = max(GC.maxptr, iptr);
    GC.minptr = min(GC.minptr, iptr);

    // do not allow to overwrite the pair
    // if this pair is already presented by this key...
    // is it possible?
    void *overwritten = map_put(GC.heap, ptr, mem);
    assert(overwritten == NULL);

    /// XXX: not sure how to do this properly...
    if (GC.bytes_allocated >= GC.limit) {
        run_gc(_file, _func, _line);
    }

    GC.bytes_allocated += size;
    GC.totally_allocated += size;
}

void* gc_alloc_mem_internal(size_t size, const char *_file, const char *_func, int _line)
{
    void *ptr = unmanaged_malloc(size);
    put_memory_header_into_the_heap(ptr, size, _file, _func, _line);
    return ptr;
}

void* gc_realloc_mem_internal(void *ptr, size_t newsize, const char *_file, const char *_func,
        int _line)
{
    assert(ptr);
    assert(newsize);

    struct gc_ptr *mem = map_remove(GC.heap, ptr);
    if (!mem) {
        cc_fatal("pointer was not found in the heap: %p\n", ptr);
    }

    assert(mem->ptr);
    assert(mem->size);

    // create a new pointer, and copy all the bytes from the old one
    // it is possible to use realloc here
    // but - using malloc is much straightforward in my opinion.
    // dunno why.
    void *newptr = unmanaged_malloc(newsize);
    memcpy(newptr, mem->ptr, mem->size);

    // manually destroy the old object
    // we've already deleted it from the heap
    // now we have to free the content
    gc_free_mem(&mem);

    // create new object, and put it into the heap
    put_memory_header_into_the_heap(newptr, newsize, _file, _func, _line);
    return newptr;
}

char* gc_strdup_internal(char *str, const char *_file, const char *_func, int _line)
{
    assert(str);
    size_t newlen = strlen(str) + 1;
    char *newstr = (char*) gc_alloc_mem_internal(newlen, _file, _func, _line);
    strcpy(newstr, str);
    return newstr;
}

static void gc_free_mem(struct gc_ptr **mem)
{
    assert(*mem);
    assert((*mem)->ptr);
    assert((*mem)->size);

    GC.bytes_allocated -= (*mem)->size;
    GC.totally_deallocated += (*mem)->size;

    free((*mem)->ptr);
    (*mem)->ptr = NULL;

    free((*mem)->location);
    (*mem)->location = NULL;

    (*mem)->size = 0;
    (*mem)->marked = 0;

    free(*mem);
    *mem = NULL;
}

static struct gc_array* gc_get_roots()
{

    void *__rsp = NULL;

    jmp_buf jb;
    setjmp(jb);

    uint8_t *rsp = (uint8_t*) &__rsp;
    uint8_t *top = (uint8_t*) GC.bottom;

    assert(rsp);
    assert(top);

    // TODO: rsp<top, rsp>top

    struct gc_array *result = array_new();
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

        struct gc_ptr *mem = map_get(GC.heap, address);
        if (mem) {
            array_add(result, mem);
        }

    }

    return result;
}

static struct gc_array* gc_get_pointers(struct gc_ptr *mem)
{
    struct gc_array *result = array_new();

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

        struct gc_ptr *mem = map_get(GC.heap, address);
        if (mem) {
            array_add(result, mem);
        }

    }

    return result;
}

static void gc_mark()
{

    struct gc_array *worklist = gc_get_roots();

    while (!array_is_empty(worklist)) {
        struct gc_ptr *mem = array_pop_back(worklist);
        if (!mem->marked) {
            mem->marked = 1;
            struct gc_array *pointers = gc_get_pointers(mem);
            for (size_t i = 0; i < pointers->size; i += 1) {
                struct gc_ptr *ptr = (struct gc_ptr*) array_get(pointers, i);
                array_add(worklist, ptr);
            }
            array_free(pointers);
        }
    }

    array_free(worklist);
}

static void gc_sweep()
{

    struct gc_array *to_remove = array_new();

    for (size_t i = 0; i < GC.heap->capacity; i++) {

        struct gc_map_ent *e = GC.heap->table[i];
        if (e == NULL) {
            continue;
        }

        for (; e; e = e->next) {
            struct gc_ptr *mem = (struct gc_ptr*) e->val;
            if (mem->marked) {
                mem->marked = 0;
            } else {
                array_add(to_remove, mem);
            }
        }
    }

    for (size_t i = 0; i < to_remove->size; i += 1) {
        struct gc_ptr *mem = (struct gc_ptr*) array_get(to_remove, i);
        void *ptr = mem->ptr;
        map_remove(GC.heap, ptr);
        gc_free_mem(&mem);
    }

    array_free(to_remove);
}

static void gc_print_heap()
{
    static char *delim =
            "|----------------|----------------|--|----------------|----------------------------------------------------------------|\n";

    printf("%s", delim);
    printf("|%16s|%16s|%2s|%16s|%64s|\n", "pointer", "object", "M", "size", "location");
    printf("%s", delim);

    for (size_t i = 0; i < GC.heap->capacity; i++) {
        struct gc_map_ent *e = GC.heap->table[i];
        if (e == NULL) {
            continue;
        }
        for (; e; e = e->next) {
            struct gc_ptr *mem = (struct gc_ptr*) e->val;
            printf("|%16lu|%16lu|%2s|%16lu|%-64s|\n", (size_t) e->key, (size_t) e->val,
                    (mem->marked ? "V" : " "), mem->size, mem->location);
            printf("%s", delim);
        }
    }

    printf("TOTAL SIZE=%lu, capacity=%lu\n", GC.heap->size, GC.heap->capacity);
}

void gc_print_stat()
{

    size_t total_time = GC.mark_msec + GC.sweep_msec;

    printf("\n------- STAT ------- \n");

    printf("TOTALLY_ALLOCATED   %lu bytes, %lu Kb, %lu Mb\n", GC.totally_allocated,
            GC.totally_allocated / GC_KB, GC.totally_allocated / GC_MB);

    printf("TOTALLY_DEALLOCATED %lu bytes, %lu Kb, %lu Mb\n", GC.totally_deallocated,
            GC.totally_deallocated / GC_KB, GC.totally_deallocated / GC_MB);

    printf("GC_INVOKED %lu times\n", GC.times_invoked);
    printf("MARK       sec:%lu, msec:%lu\n", GC.mark_msec / 1000, GC.mark_msec % 1000);
    printf("SWEEP      sec:%lu, msec:%lu\n", GC.sweep_msec / 1000, GC.sweep_msec % 1000);
    printf("TOTAL      sec:%lu, msec:%lu\n", total_time / 1000, total_time % 1000);

    printf("\nHEAP NOW \n");
    gc_print_heap();
}

int do_main(int argc, char **argv)
{
    char *str = gc_malloc(1024);
    str = gc_realloc(str, 2048);

    for (int i = 0; i < 8; i += 1) {
        str = gc_malloc(32768);
    }

    gc_print_stat();
    printf("\n:ok:\n");
    return 0;
}

int main(int argc, char **argv)
{
    gc_init(&GC, &argc);
    return do_main(argc, argv);
}

