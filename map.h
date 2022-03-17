#ifndef SCLIB_HASHMAP_H
#define SCLIB_HASHMAP_H

#include <stddef.h>
#include <stdint.h>

#include "fnptr.h"

typedef struct hashmap HashMap;
typedef struct entry Entry;

struct entry {
    void* key;
    void* val;
    Entry* next;
};

struct hashmap {
    hash_fn hash;
    equal_fn equal;
    size_t size;
    size_t capacity;
    Entry** table;
    size_t threshold;
};

HashMap* HashMap_new(hash_fn hash, equal_fn equal);
HashMap* HashMap_new_str(void);
HashMap* HashMap_new_int(void);
void HashMap_free(HashMap* self);
size_t HashMap_size(HashMap* self);
int HashMap_empty(HashMap* self);
void* HashMap_get(HashMap* self, void* key);
void* HashMap_put(HashMap* self, void* key, void* value);
void* HashMap_remove(HashMap* self, void* key);
void HashMap_clear(HashMap* self);
void** HashMap_keys(HashMap* self);
int HashMap_contains_key(HashMap *self, void *key);

size_t hashmap_hash_str(void* key);
int hashmap_equal_str(void* key1, void* key2);

size_t hashmap_hash_int(void* key);
int hashmap_equal_int(void* key1, void* key2);

size_t hashmap_hash_ptr(void *ptr);
int hashmap_equal_ptr(void *a, void *b);

#endif /* SCLIB_HASHMAP_H */
