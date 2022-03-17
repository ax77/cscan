#ifndef FNPTR_H_
#define FNPTR_H_

#include <stddef.h>

typedef size_t (*hash_fn)(void* key);
typedef int (*equal_fn)(void* a, void* b);
typedef int (*cmp_fn)(void* a, void* b);
typedef void (*free_fn)(void* ptr);

#endif /* FNPTR_H_ */
