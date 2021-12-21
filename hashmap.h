#ifndef SCLIB_HASHMAP_H
#define SCLIB_HASHMAP_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct hashmap HashMap;
typedef struct entry Entry;

typedef size_t (*HashMap_HashFunc)(void* key);
typedef int (*HashMap_EqualFunc)(void* key1, void* key2);

struct entry {
    void* key;
    void* val;
    Entry* next;
};

struct hashmap {
    HashMap_HashFunc hash;
    HashMap_EqualFunc equal;
    size_t size;
    size_t capacity;
    Entry** table;
    int threshold;
};

HashMap* HashMap_new(HashMap_HashFunc hash, HashMap_EqualFunc equal);
HashMap* HashMap_new_str(void);
HashMap* HashMap_new_int(void);
void HashMap_free(HashMap* self);
size_t HashMap_size(HashMap* self);
bool HashMap_empty(HashMap* self);
void* HashMap_get(HashMap* self, void* key);
void* HashMap_add(HashMap* self, void* key, void* value);
void* HashMap_remove(HashMap* self, void* key);
void HashMap_clear(HashMap* self);
void** HashMap_keys(HashMap* self);

// For debug
void HashMap_print_strstr(HashMap* self);

#endif /* SCLIB_HASHMAP_H */
