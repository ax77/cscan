#ifndef C_ARRAY_H_
#define C_ARRAY_H_

#include "cheaders.h"

typedef struct array_list ArrayList;

struct array_list {
    void **table;
    size_t len, alloc;
};

ArrayList *ArrayList_new();
void ArrayList_add(ArrayList *list, void *what);
void *ArrayList_pop_back(ArrayList *list);
void *ArrayList_get(ArrayList *list, size_t index);

#endif /* C_ARRAY_H_ */
