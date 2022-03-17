#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "xmem.h"

static const size_t kDefaultCapacity = 11;
static const float kLoadFactor = 0.75;

static void* entry_new(void* key, void* val, Entry* next)
{
    Entry* entry = (Entry*) cc_malloc(sizeof(Entry));
    entry->key = key;
    entry->val = val;
    entry->next = next;
    return entry;
}

static Entry **empty_table(size_t capacity)
{
    assert(capacity);

    Entry **table = (Entry**) cc_malloc(sizeof(Entry*) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        table[i] = NULL;
    }
    return table;
}

HashMap* HashMap_new(hash_fn hash, equal_fn equal)
{
    assert(hash);
    assert(equal);

    HashMap* hashmap = (HashMap*) cc_malloc(sizeof(HashMap));
    hashmap->hash = hash;
    hashmap->equal = equal;
    hashmap->size = 0;
    hashmap->capacity = kDefaultCapacity;
    hashmap->table = empty_table(hashmap->capacity);
    hashmap->threshold = hashmap->capacity * kLoadFactor;
    return hashmap;
}

HashMap* HashMap_new_str(void)
{
    return HashMap_new(hashmap_hash_str, hashmap_equal_str);
}

HashMap* HashMap_new_int(void)
{
    return HashMap_new(hashmap_hash_int, hashmap_equal_int);
}

void HashMap_free(HashMap* self)
{
    assert(self);

    HashMap_clear(self);
    free(self->table);
    free(self);
}

size_t HashMap_size(HashMap* self)
{
    assert(self);
    return self->size;
}

int HashMap_empty(HashMap* self)
{
    assert(self);
    return self->size == 0;
}

static size_t hashmap_index(HashMap* self, void* key, size_t capacity)
{
    return self->hash(key) % capacity;
}

void* HashMap_get(HashMap* self, void* key)
{
    assert(self);
    assert(key);

    size_t index = hashmap_index(self, key, self->capacity);
    for (Entry* e = self->table[index]; e; e = e->next) {
        if (self->equal(e->key, key)) {
            return e->val;
        }
    }
    return NULL;
}

int HashMap_contains_key(HashMap *self, void *key)
{
    assert(self);
    assert(key);
    return HashMap_get(self, key) != NULL;
}

static void hashmap_rehash(HashMap* self, Entry** new_table, size_t new_capacity)
{
    for (size_t i = 0; i < self->capacity; i++) {
        Entry* next = NULL;
        for (Entry* e = self->table[i]; e; e = next) {
            next = e->next;
            size_t index = hashmap_index(self, e->key, new_capacity);
            e->next = new_table[index];
            new_table[index] = e;
        }
    }

    free(self->table);
    self->table = new_table;
    self->capacity = new_capacity;
}

static void hashmap_resize(HashMap* self)
{
    size_t new_capacity = self->capacity * 2 + 1;
    Entry** new_table = empty_table(new_capacity);
    hashmap_rehash(self, new_table, new_capacity);
    self->threshold = new_capacity * kLoadFactor;
}

void* HashMap_put(HashMap* self, void* key, void* val)
{
    assert(self);
    assert(key);

    size_t index = hashmap_index(self, key, self->capacity);

    // overwrite
    for (Entry* e = self->table[index]; e; e = e->next) {
        if (self->equal(key, e->key)) {
            void *oldval = e->val;
            e->val = val;
            return oldval;
        }
    }

    // resize
    if (self->size > self->threshold) {
        hashmap_resize(self);
        index = hashmap_index(self, key, self->capacity);
    }

    // add
    Entry* new_entry = entry_new(key, val, self->table[index]);
    self->table[index] = new_entry;
    self->size++;
    return NULL;
}

void* HashMap_remove(HashMap* self, void* key)
{
    assert(self);
    assert(key);

    size_t index = hashmap_index(self, key, self->capacity);
    Entry* e = self->table[index];
    if (e == NULL) {
        return NULL;
    }

    Entry* prev = NULL;
    Entry* next = NULL;
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

void HashMap_clear(HashMap* self)
{
    assert(self);

    for (size_t i = 0; i < self->capacity; i++) {
        Entry* e = self->table[i];
        if (e == NULL) {
            continue;
        }
        Entry* next;
        for (; e; e = next) {
            next = e->next;
            free(e);
        }
        self->table[i] = NULL;
    }
    self->size = 0;
}

void** HashMap_keys(HashMap* self)
{
    assert(self);

    void** keys = (void**) cc_malloc(sizeof(void*) * (self->size + 1));
    keys[self->size] = NULL;
    size_t count = 0;
    for (size_t i = 0; i < self->capacity; i++) {
        Entry* e = self->table[i];
        if (e == NULL) {
            continue;
        }
        for (; e; e = e->next) {
            keys[count] = e->key;
            count++;
        }
    }
    return keys;
}

size_t hashmap_hash_str(void* key)
{
    unsigned char* str = key;

    // djb2
    size_t hash = 5381;
    size_t c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash;
}

int hashmap_equal_str(void* key1, void* key2)
{
    return strcmp((char*) key1, (char*) key2) == 0;
}

size_t hashmap_hash_int(void* key)
{
    return *((size_t*) key);
}

int hashmap_equal_int(void* key1, void* key2)
{
    return *((size_t*) key1) == *((size_t*) key2);
}

size_t hashmap_hash_ptr(void *ptr)
{
    return (size_t) ptr;
}

int hashmap_equal_ptr(void *a, void *b)
{
    return a == b;
}

