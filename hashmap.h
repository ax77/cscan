#ifndef SCLIB_HASHMAP_H
#define SCLIB_HASHMAP_H

#include <stdbool.h>
#include <stdint.h>

typedef struct hashmap HashMap;

HashMap* HashMap_new();
void HashMap_free(HashMap* self);
int HashMap_size(HashMap* self);
bool HashMap_empty(HashMap* self);
void* HashMap_get(HashMap* self, void* key);
void* HashMap_add(HashMap* self, void* key, void* value);
void* HashMap_remove(HashMap* self, void* key);
void HashMap_clear(HashMap* self);
void** HashMap_keys(HashMap* self);

#endif /* SCLIB_HASHMAP_H */
