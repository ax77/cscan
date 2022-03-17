#ifndef VEC_H_
#define VEC_H_

#include "cheaders.h"
#include "fnptr.h"

typedef struct vec vec;
struct vec {
    void **data;
    size_t size, alloc;
};

vec *vec_new();
void vec_push(vec *v, void *p);
void *vec_pop(vec *v);
void *vec_get(vec *v, size_t at);
void *vec_set(vec *v, size_t at, void *p);

#endif /* VEC_H_ */
