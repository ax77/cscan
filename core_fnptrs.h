#ifndef CORE_FNPTRS_H_
#define CORE_FNPTRS_H_

#include <stddef.h>

typedef size_t (*hash_fn)(void* key);
typedef int (*equal_fn)(void* a, void* b);
typedef int (*cmp_fn)(void* a, void* b);

#endif /* CORE_FNPTRS_H_ */
