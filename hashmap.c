#include "hashmap.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int kDefaultCapacity = 11;
static const float kLoadFactor = 0.75;

typedef struct entry Entry;
struct entry {
    void* key;
    void* val;
    Entry* next;
};

struct hashmap {
    int size;
    int capacity;
    Entry** table;
    int threshold;
};

static uint32_t hashmap_hash_str(void* key) {
    unsigned char* str = key;

    // djb2
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash;
}

static bool hashmap_equal_str(void* key1, void* key2) {
    return strcmp((char*) key1, (char*) key2) == 0;
}

static void* entry_new(void* key, void* val, Entry* next) {
    Entry* entry = (Entry*) malloc(sizeof(Entry));
    if (entry == NULL) {
        fprintf(stderr, "ERROR: out of memory\n");
        return NULL;
    }
    entry->key = key;
    entry->val = val;
    entry->next = next;
    return entry;
}

HashMap* HashMap_new() {
    HashMap* hashmap = (HashMap*) malloc(sizeof(HashMap));
    if (hashmap == NULL) {
        fprintf(stderr, "ERROR: out of memory\n");
        return NULL;
    }
    hashmap->size = 0;
    hashmap->capacity = kDefaultCapacity;
    hashmap->table = (Entry**) malloc(sizeof(Entry*) * hashmap->capacity);
    if (hashmap->table == NULL) {
        fprintf(stderr, "ERROR: out of memory\n");
        return NULL;
    }
    for (int i = 0; i < hashmap->capacity; i++) {
        hashmap->table[i] = NULL;
    }
    hashmap->threshold = hashmap->capacity * kLoadFactor;
    return hashmap;
}

void HashMap_free(HashMap* self) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return;
    }

    HashMap_clear(self);
    free(self->table);
    free(self);
}

int HashMap_size(HashMap* self) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return 0;
    }

    return self->size;
}

bool HashMap_empty(HashMap* self) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return false;
    }

    return self->size == 0;
}

static int hashmap_index(HashMap* self, void* key, int capacity) {
    return hashmap_hash_str(key) % capacity;
}

void* HashMap_get(HashMap* self, void* key) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return NULL;
    }
    if (key == NULL) {
        assert(false && "key must not be null");
        return NULL;
    }

    int index = hashmap_index(self, key, self->capacity);
    for (Entry* e = self->table[index]; e; e = e->next) {
        if (hashmap_equal_str(e->key, key)) {
            return e->val;
        }
    }
    return NULL;
}

static void hashmap_rehash(HashMap* self, Entry** newTable, int newCapacity) {
    for (int i = 0; i < newCapacity; i++) {
        newTable[i] = NULL;
    }
    for (int i = 0; i < self->capacity; i++) {
        Entry* next;
        for (Entry* e = self->table[i]; e; e = next) {
            next = e->next;
            int index = hashmap_index(self, e->key, newCapacity);
            e->next = newTable[index];
            newTable[index] = e;
        }
    }
    free(self->table);
    self->table = newTable;
    self->capacity = newCapacity;
}

static bool hashmap_resize(HashMap* self) {
    int newCapacity = self->capacity * 2 + 1;
    Entry** newTable = (Entry**) malloc(sizeof(Entry*) * newCapacity);
    if (newTable == NULL) {
        fprintf(stderr, "ERROR: out of memory\n");
        return false;
    }
    hashmap_rehash(self, newTable, newCapacity);
    self->threshold = newCapacity * kLoadFactor;
    return true;
}

void* HashMap_add(HashMap* self, void* key, void* val) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return NULL;
    }
    if (key == NULL) {
        assert(false && "key must not be null");
        return NULL;
    }

    int index = hashmap_index(self, key, self->capacity);
    // overwrite
    for (Entry* e = self->table[index]; e; e = e->next) {
        if (hashmap_equal_str(key, e->key)) {
            e->val = val;
            return val;
        }
    }
    // resize
    if (self->size > self->threshold) {
        if (!hashmap_resize(self)) {
            return NULL;
        }
        index = hashmap_index(self, key, self->capacity);
    }
    // add
    Entry* newEntry = entry_new(key, val, self->table[index]);
    if (newEntry == NULL) {
        return NULL;
    }
    self->table[index] = newEntry;
    self->size++;
    return val;
}

void* HashMap_remove(HashMap* self, void* key) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return NULL;
    }
    if (key == NULL) {
        assert(false && "key must not be null");
        return NULL;
    }

    int index = hashmap_index(self, key, self->capacity);
    Entry* e = self->table[index];
    if (e == NULL) {
        return NULL;
    }
    Entry* prev = NULL;
    Entry* next;
    for (; e; prev = e, e = next) {
        next = e->next;
        if (hashmap_equal_str(key, e->key)) {
            void* val = e->val;
            if (prev == NULL)
                self->table[index] = next;
            else
                prev->next = next;
            self->size--;
            free(e);
            return val;
        }
    }
    return NULL;
}

void HashMap_clear(HashMap* self) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return;
    }

    for (int i = 0; i < self->capacity; i++) {
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

void** HashMap_keys(HashMap* self) {
    if (self == NULL) {
        assert(false && "self must not be null");
        return NULL;
    }

    void** keys = (void**) malloc(sizeof(void*) * (self->size + 1));
    if (keys == NULL) {
        fprintf(stderr, "ERROR: out of memory\n");
        return NULL;
    }
    keys[self->size] = NULL;
    int count = 0;
    for (int i = 0; i < self->capacity; i++) {
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

//
// For debug
//

static void hashmap_print(HashMap* self, void (*printFunc)(Entry*)) {
    printf("HashMap: size=%d, capacity=%d\n", self->size, self->capacity);
    for (int i = 0; i < self->capacity; i++) {
        Entry* e = self->table[i];
        if (e == NULL) {
            continue;
        }
        printf("%4d: ", i);
        for (; e; e = e->next) {
            printFunc(e);
        }
        printf("\n");
    }
}

static void entry_print_strstr(Entry* self) {
    printf("[key=%s, val=%s] -> ", (char*) self->key, (char*) self->val);
}

void HashMap_print_strstr(HashMap* self) {
    hashmap_print(self, entry_print_strstr);
}
