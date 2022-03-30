#include "map.h"

map_impl(char*, int, str_i32);

size_t hashmap_hash_str(char *key)
{
    unsigned char *str = (unsigned char*) key;

    // djb2
    size_t hash = 5381;
    size_t c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash;
}

int hashmap_equal_str(char *key1, char *key2)
{
    return strcmp(key1, key2) == 0;
}

size_t hashmap_hash_int(int key)
{
    return ((size_t) key);
}

int hashmap_equal_int(int key1, int key2)
{
    return key1 == key2;
}

size_t hashmap_hash_ptr(void *ptr)
{
    return (size_t) ptr;
}

int hashmap_equal_ptr(void *a, void *b)
{
    return a == b;
}

