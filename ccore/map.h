#ifndef CCORE_MAP_H_
#define CCORE_MAP_H_

#include "xmem.h"

#define MAP_INIT(NAME, hash, equal) { \
      .hash_fn = hash  \
    , .equal_fn = equal   \
    , .size = 0                         \
    , .capacity = 0                        \
    , .threshold = 0                        \
    , .table = NULL                        \
    , .functions = &(map_functions_impl_##NAME) }

#define map_proto(KTYPE, VTYPE, NAME)                                                \
                                                                                     \
    typedef struct entry_##NAME      map_entry_##NAME;                                   \
    typedef struct hashmap_##NAME    map_##NAME;                                         \
    typedef struct map_result_##NAME map_result_##NAME;                                  \
    struct map_functions_##NAME      map_functions_impl_##NAME;                          \
                                                                                     \
    static const size_t MAP_DEFAULT_CAPACITY_##NAME = 11;                            \
    static const float MAP_LOAD_FACTOR_##NAME = 0.75f;                                \
                                                                                     \
    struct entry_##NAME {                                                                \
        KTYPE key;                                                                       \
        VTYPE val;                                                                       \
        struct entry_##NAME* next;                                                       \
    };                                                                                   \
                                                                                         \
    struct hashmap_##NAME {                                                              \
        size_t (*hash_fn)(KTYPE key);                                                    \
        int (*equal_fn)(KTYPE a, KTYPE b);                                               \
        struct map_functions_##NAME *functions;                                          \
        size_t size;                                                                     \
        size_t capacity;                                                                 \
        struct entry_##NAME** table;                                                     \
        size_t threshold;                                                                \
    };                                                                                   \
                                                                                         \
    struct map_result_##NAME {                                                           \
        VTYPE value;                                                                     \
        int found;                                                                       \
    };                                                                                   \
                                                                                         \
    struct map_functions_##NAME {                                                        \
       struct map_result_##NAME (*map_put)                                               \
           (struct hashmap_##NAME* self, KTYPE key, VTYPE val);                          \
                                                                                         \
       struct map_result_##NAME (*map_get)                                               \
           (struct hashmap_##NAME* self, KTYPE key);                                     \
                                                                                         \
       struct map_result_##NAME (*map_remove)                                            \
           (struct hashmap_##NAME* self, KTYPE key);                                     \
    };                                                                                   \
                                                                                         \
    struct hashmap_##NAME*                                                               \
    map_new_##NAME(size_t (*hash_fn)(KTYPE key), int (*equal_fn)(KTYPE a, KTYPE b));     \
                                                                                         \
    struct map_result_##NAME                                                             \
    map_put_##NAME(struct hashmap_##NAME* self, KTYPE key, VTYPE val);                   \
                                                                                         \
    struct map_result_##NAME                                                             \
    map_get_##NAME(struct hashmap_##NAME* self, KTYPE key);                              \
                                                                                         \
    struct map_result_##NAME                                                             \
    map_remove_##NAME(struct hashmap_##NAME *self, KTYPE key);

#define map_impl(KTYPE, VTYPE, NAME)                                                 \
                                                                                     \
struct map_functions_##NAME map_functions_impl_##NAME =                              \
{                                                                                    \
    .map_put = &map_put_##NAME,                                                      \
    .map_get = &map_get_##NAME,                                                      \
    .map_remove = &map_remove_##NAME,                                                \
};                                                                                   \
                                                                                     \
static size_t                                                                        \
map_index_##NAME(struct hashmap_##NAME* self, KTYPE key, size_t capacity)                \
{                                                                                    \
    return self->hash_fn(key) % capacity;                                            \
}                                                                                    \
                                                                                     \
static struct entry_##NAME *                                                         \
map_entry_new_##NAME(KTYPE key, VTYPE val, struct entry_##NAME* next)                    \
{                                                                                    \
    struct entry_##NAME* entry =                                                     \
        (struct entry_##NAME*)                                                       \
            cc_malloc(sizeof(struct entry_##NAME));                                    \
    entry->key = key;                                                                \
    entry->val = val;                                                                \
    entry->next = next;                                                              \
    return entry;                                                                    \
}                                                                                    \
                                                                                     \
static struct entry_##NAME **                                                        \
map_empty_table_##NAME(size_t capacity)                                                  \
{                                                                                    \
    assert(capacity);                                                                \
                                                                                     \
    struct entry_##NAME **table =                                                    \
        (struct entry_##NAME**)                                                      \
            cc_malloc(sizeof(struct entry_##NAME*) * capacity);                        \
    for (size_t i = 0; i < capacity; i++) {                                          \
        table[i] = NULL;                                                             \
    }                                                                                \
    return table;                                                                    \
}                                                                                    \
\
static void                                                                  \
map_init_table_##NAME(struct hashmap_##NAME *hashmap)                        \
{                                                                            \
    assert(hashmap);                                                         \
    hashmap->size = 0;                                                       \
    hashmap->capacity = MAP_DEFAULT_CAPACITY_##NAME;                         \
    hashmap->table = map_empty_table_##NAME(hashmap->capacity);              \
    hashmap->threshold = hashmap->capacity * MAP_LOAD_FACTOR_##NAME;         \
}                                                                            \
                                                                                     \
struct hashmap_##NAME*                                                               \
map_new_##NAME(size_t (*hash_fn)(KTYPE key), int (*equal_fn)(KTYPE a, KTYPE b))      \
{                                                                                    \
    assert(hash_fn);                                                                 \
    assert(equal_fn);                                                                \
                                                                                     \
    struct hashmap_##NAME* hashmap =                                                 \
        (struct hashmap_##NAME*)                                                     \
            cc_malloc(sizeof(struct hashmap_##NAME));                                  \
    hashmap->hash_fn = hash_fn;                                                      \
    hashmap->equal_fn = equal_fn;                                                    \
    hashmap->functions = &map_functions_impl_##NAME;                                 \
    map_init_table_##NAME(hashmap);                                                  \
    return hashmap;                                                                  \
}                                                                                    \
                                                                                     \
struct map_result_##NAME                                                             \
map_put_##NAME(struct hashmap_##NAME* self, KTYPE key, VTYPE val)                    \
{                                                                                    \
    assert(self);                                                                    \
    assert(key);                                                                     \
                                         \
    if(self->capacity == 0) {            \
        assert(self->size == 0);         \
        assert(self->threshold == 0);    \
        assert(self->table == NULL);     \
        assert(self->functions != NULL); \
        assert(self->hash_fn != NULL);  \
        assert(self->equal_fn != NULL); \
                                     \
        map_init_table_##NAME(self); \
    }                                \
                                                                                     \
    size_t index = map_index_##NAME(self, key, self->capacity);                          \
                                                                                     \
    for (struct entry_##NAME* e = self->table[index]; e; e = e->next) {              \
        if (self->equal_fn(key, e->key)) {                                           \
            VTYPE oldval = e->val;                                                   \
            e->val = val;                                                            \
            struct map_result_##NAME result = { .value = oldval, .found = 1 };       \
            return result;                                                           \
        }                                                                            \
    }                                                                                \
                                                                                     \
    if (self->size >= self->threshold) {                                             \
                                                                                     \
        size_t new_capacity = self->capacity * 2 + 1;                                \
        struct entry_##NAME** new_table = map_empty_table_##NAME(new_capacity);          \
        for (size_t i = 0; i < self->capacity; i++) {                                \
            struct entry_##NAME* next = NULL;                                        \
            for (struct entry_##NAME* e = self->table[i]; e; e = next) {             \
                next = e->next;                                                      \
                size_t index = map_index_##NAME(self, e->key, new_capacity);             \
                e->next = new_table[index];                                          \
                new_table[index] = e;                                                \
            }                                                                        \
        }                                                                            \
        cc_free(&(self->table));                                                           \
        self->table = new_table;                                                     \
        self->capacity = new_capacity;                                               \
        self->threshold = new_capacity * MAP_LOAD_FACTOR_##NAME;                     \
                                                                                     \
        index = map_index_##NAME(self, key, self->capacity);                             \
    }                                                                                \
                                                                                     \
    struct entry_##NAME* new_entry = map_entry_new_##NAME(                               \
          key                                                                        \
        , val                                                                        \
        , self->table[index]);                                                       \
                                                                                     \
    self->table[index] = new_entry;                                                  \
    self->size++;                                                                    \
                                                                                     \
    struct map_result_##NAME result = { .value = ((VTYPE) 0), .found = 0 };          \
    return result;                                                                   \
}                                                                                    \
                                                                                     \
struct map_result_##NAME                                                             \
map_get_##NAME(struct hashmap_##NAME* self, KTYPE key)                               \
{                                                                                    \
    assert(self);                                                                    \
    assert(key);                                                                     \
                                                                                     \
    size_t index = map_index_##NAME(self, key, self->capacity);                          \
    for (struct entry_##NAME* e = self->table[index]; e; e = e->next) {              \
        if (self->equal_fn(e->key, key)) {                                           \
            struct map_result_##NAME result = { .value = e->val, .found = 1 };       \
            return result;                                                           \
        }                                                                            \
    }                                                                                \
                                                                                     \
    struct map_result_##NAME result = { .value = ((VTYPE) 0), .found = 0 };          \
    return result;                                                                   \
}                                                                                    \
                                                                                     \
struct map_result_##NAME                                                             \
map_remove_##NAME(struct hashmap_##NAME *self, KTYPE key)                            \
{                                                                                    \
    assert(self);                                                                    \
    assert(key);                                                                     \
                                                                                     \
    size_t index = map_index_##NAME(self, key, self->capacity);                          \
    struct entry_##NAME *e = self->table[index];                                     \
    if (e == NULL) {                                                                 \
        struct map_result_##NAME result = { .value = ((VTYPE)0), .found = 0 };       \
        return result;                                                               \
    }                                                                                \
                                                                                     \
    struct entry_##NAME *prev = NULL;                                                \
    struct entry_##NAME *next = NULL;                                                \
    for (; e; prev = e, e = next) {                                                  \
        next = e->next;                                                              \
        if (self->equal_fn(key, e->key)) {                                           \
            VTYPE val = e->val;                                                      \
            if (prev == NULL) {                                                      \
                self->table[index] = next;                                           \
            } else {                                                                 \
                prev->next = next;                                                   \
            }                                                                        \
            self->size -= 1;                                                         \
            cc_free(&e);                                                                 \
                                                                                     \
            struct map_result_##NAME result = { .value = val, .found = 1 };          \
            return result;                                                           \
        }                                                                            \
    }                                                                                \
                                                                                     \
    struct map_result_##NAME result = { .value = ((VTYPE)0), .found = 0 };           \
    return result;                                                                   \
}

#define map(name) map_##name
#define map_new(name, h, e) map_new_##name(h, e)
#define map_put(container, k, v) (container)->functions->map_put(container, k, v)
#define map_get(container, k) (container)->functions->map_get(container, k)
#define map_remove(container, k) (container)->functions->map_remove(container, k)
#define map_result(name) map_result_##name

size_t hashmap_hash_str(char* key);
int hashmap_equal_str(char* key1, char* key2);

size_t hashmap_hash_int(int key);
int hashmap_equal_int(int key1, int key2);

size_t hashmap_hash_ptr(void *ptr);
int hashmap_equal_ptr(void *a, void *b);

map_proto(char*, int, str_i32);

#endif /* CCORE_MAP_H_ */
