#ifndef VEC3_H_
#define VEC3_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include "xmem.h"

#define VEC_INIT(NAME) { .data = NULL   \
    , .size = 0                         \
    , .alloc = 0                        \
    , .functions = &(vec_functions_impl_##NAME) }

#define vec_proto(TYPE, NAME)                                                 \
                                                                              \
typedef struct vec_##NAME vec_##NAME;                                         \
typedef struct vec_functions_##NAME vec_functions_##NAME;                     \
struct vec_functions_##NAME vec_functions_impl_##NAME;                        \
                                                                              \
struct vec_##NAME {                                                           \
    TYPE * data;                                                              \
    size_t size, alloc;                                                       \
    vec_functions_##NAME *functions;                                          \
};                                                                            \
                                                                              \
vec_##NAME * vec_new_##NAME();                                                \
void         vec_push_      ##NAME   (vec_##NAME *v, TYPE p);                 \
TYPE         vec_pop_       ##NAME   (vec_##NAME *v);                         \
TYPE         vec_get_       ##NAME   (vec_##NAME *v, size_t index);           \
TYPE         vec_set_       ##NAME   (vec_##NAME *v, size_t index, TYPE p);   \
size_t       vec_size_      ##NAME   (vec_##NAME *v);                         \
int          vec_is_empty_  ##NAME   (vec_##NAME *v);                         \
void         vec_add_all_   ##NAME   (vec_##NAME *dst, vec_##NAME *src);      \
void         vec_reset_     ##NAME   (vec_##NAME *v);                         \
                                                                              \
struct vec_functions_##NAME {                                                 \
    void   (*push_back) (vec_##NAME *v, TYPE p);                              \
    TYPE   (*pop_back)  (vec_##NAME *v);                                      \
    TYPE   (*get)       (vec_##NAME *v, size_t index);                        \
    TYPE   (*set)       (vec_##NAME *v, size_t index, TYPE p);                \
    size_t (*size)      (vec_##NAME *v);                                      \
    int    (*is_empty)  (vec_##NAME *v);                                      \
    void   (*add_all)   (vec_##NAME *dst, vec_##NAME *src);                   \
    void   (*reset)     (vec_##NAME *v);                                      \
};

#define vec_impl(TYPE, NAME)                                                  \
                                                                              \
struct vec_functions_##NAME vec_functions_impl_##NAME = {                     \
    .push_back = &vec_push_     ##NAME,                                       \
    .pop_back  = &vec_pop_      ##NAME,                                       \
    .get       = &vec_get_      ##NAME,                                       \
    .set       = &vec_set_      ##NAME,                                       \
    .size      = &vec_size_     ##NAME,                                       \
    .is_empty  = &vec_is_empty_ ##NAME,                                       \
    .add_all   = &vec_add_all_  ##NAME,                                       \
    .reset     = &vec_reset_    ##NAME,                                       \
};                                                                            \
                                                                              \
vec_##NAME* vec_new_##NAME()                                                  \
{                                                                             \
    struct vec_##NAME *v = cc_malloc(sizeof(struct vec_##NAME));              \
    v->size = 0;                                                              \
    v->alloc = 2;                                                             \
    v->data = (TYPE *) cc_malloc(v->alloc * sizeof(TYPE));                    \
    v->functions = &vec_functions_impl_##NAME;                                \
    return v;                                                                 \
}                                                                             \
                                                                              \
static void vec_grow_1##NAME(vec_##NAME *v)                                   \
{                                                                             \
    assert(v);                                                                \
    assert(v->size < INT_MAX);                                                \
    assert(v->alloc < INT_MAX);                                               \
    if (v->alloc == 0) {                                                      \
        assert(v->data == NULL);                                              \
        assert(v->size == 0);                                                 \
        v->alloc = 2;                                                         \
        v->data = (TYPE *) cc_malloc(v->alloc * sizeof(TYPE));                \
    } else {                                                                  \
        assert(v->alloc);                                                     \
        assert(v->data);                                                      \
        if ((v->size + 2) >= v->alloc) {                                      \
            v->alloc += 2;                                                    \
            v->alloc *= 2;                                                    \
            v->data = (TYPE *) cc_realloc(v->data, v->alloc * sizeof(TYPE));  \
                                                                              \
            for(size_t i = v->size; i < v->alloc; i += 1) {                   \
                v->data[i] = ((TYPE) 0);                                      \
            }                                                                 \
        }                                                                     \
    }                                                                         \
}                                                                             \
                                                                              \
void vec_push_##NAME(vec_##NAME *v, TYPE p)                                   \
{                                                                             \
    assert(v);                                                                \
    vec_grow_1##NAME(v);                                                      \
    v->data[v->size] = p;                                                     \
    v->size += 1;                                                             \
                                                                              \
    v->data[v->size] = ((TYPE) 0);                                            \
}                                                                             \
                                                                              \
TYPE vec_pop_##NAME(vec_##NAME *v)                                            \
{                                                                             \
    assert(v);                                                                \
    assert(v->data);                                                          \
    assert(v->size);                                                          \
    TYPE last = v->data[v->size - 1];                                         \
    v->size -= 1;                                                             \
                                                                              \
    v->data[v->size] = ((TYPE) 0);                                            \
    return last;                                                              \
}                                                                             \
                                                                              \
TYPE vec_get_##NAME(vec_##NAME *v, size_t index)                              \
{                                                                             \
    assert(v);                                                                \
    assert(v->data);                                                          \
    assert(index < v->size);                                                  \
    return v->data[index];                                                    \
}                                                                             \
                                                                              \
TYPE vec_set_##NAME(vec_##NAME *v, size_t index, TYPE p)                      \
{                                                                             \
    assert(v);                                                                \
    assert(v->data);                                                          \
    assert(index < v->size);                                                  \
    TYPE old = v->data[index];                                                \
    v->data[index] = p;                                                       \
    return old;                                                               \
}                                                                             \
                                                                              \
size_t vec_size_##NAME(vec_##NAME *v)                                         \
{                                                                             \
    assert(v);                                                                \
    return v->size;                                                           \
}                                                                             \
                                                                              \
int vec_is_empty_##NAME(vec_##NAME *v)                                        \
{                                                                             \
    assert(v);                                                                \
    return v->size == 0;                                                      \
}                                                                             \
                                                                              \
void vec_add_all_##NAME(vec_##NAME *dst, vec_##NAME *src)                     \
{                                                                             \
    assert(dst);                                                              \
    assert(src);                                                              \
                                                                              \
    for (size_t i = 0; i < vec_size_##NAME(src); i += 1) {                    \
        vec_push_##NAME(dst, vec_get_##NAME(src, i));                         \
    }                                                                         \
}                                                                             \
                                                                              \
void vec_reset_##NAME(vec_##NAME *v)                                          \
{                                                                             \
    assert(v);                                                                \
                                                                              \
    v->size = 0;                                                              \
    v->alloc = 0;                                                             \
    v->data = NULL;                                                           \
}

#define vec(name) vec_##name
#define vec_new(name) vec_new_##name()
#define vec_push_back(container, elem) (container)->functions->push_back(container, elem)
#define vec_pop_back(container) (container)->functions->pop_back(container)
#define vec_get(container, index) (container)->functions->get(container, index)
#define vec_set(container, index, elem) (container)->functions->set(container, index, elem)
#define vec_size(container) (container)->functions->size(container)
#define vec_is_empty(container) (container)->functions->is_empty(container)
#define vec_add_all(container, src) (container)->functions->add_all(container, src)
#define vec_reset(container) (container)->functions->reset(container)

vec_proto(char, i8)
vec_proto(unsigned char, u8)
vec_proto(char*, str)
vec_proto(unsigned int, u32);

#endif /* VEC3_H_ */

