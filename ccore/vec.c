#include "vec.h"
#include "xmem.h"

vec *vec_new()
{
    struct vec *v = cc_malloc(sizeof(struct vec));
    v->size = 0;
    v->alloc = 2;
    v->data = cc_malloc(v->alloc * sizeof(void*));
    return v;
}

static void vec_grow(vec *v)
{
    assert(v);
    assert(v->data);
    if (v->size >= v->alloc) {
        v->alloc += 2;
        v->alloc *= 2;
        v->data = cc_realloc(v->data, v->alloc * sizeof(void*));
    }
}

void vec_push(vec *v, void *p)
{
    assert(v);
    assert(v->data);
    vec_grow(v);
    v->data[v->size++] = p;
}

void *vec_pop(vec *v)
{
    assert(v);
    assert(v->data);
    assert(v->size);
    void *p = v->data[v->size - 1];
    v->size--;
    return p;
}

void *vec_get(vec *v, size_t at)
{
    assert(v);
    assert(v->data);
    assert(at < v->size);
    return v->data[at];
}

void *vec_set(vec *v, size_t at, void *p)
{
    assert(v);
    assert(v->data);
    assert(at < v->size);
    void *old = v->data[at];
    v->data[at] = p;
    return old;
}
