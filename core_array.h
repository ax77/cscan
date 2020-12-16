#ifndef CORE_ARRAY_H_
#define CORE_ARRAY_H_

#include "cheaders.h"

typedef struct array_list ArrayList;

struct array_list {
    void ** data;
    size_t size, alloc;
};

ArrayList * array_new();
void array_add(ArrayList *array, void *ptr);
void *array_get(ArrayList *array, size_t at_index);

#endif /* CORE_ARRAY_H_ */
