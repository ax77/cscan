#include "core_array.h"
#include "core_mem.h"

ArrayList * array_new(free_fn free_fn)
{
    assert(free_fn);

    ArrayList *rv = cc_malloc(sizeof(ArrayList));
    rv->alloc = 2;
    rv->size = 0;
    rv->data = cc_malloc(sizeof(void*) * rv->alloc);
    for (size_t i = 0; i < rv->alloc; i++) {
        rv->data[i] = NULL;
    }

    rv->free_fn = free_fn;
    return rv;
}

void array_add(ArrayList *array, void *ptr)
{
    assert(array);

    if (array->size >= array->alloc) {
        array->alloc *= 2;
        array->data = cc_realloc(array->data, sizeof(void*) * array->alloc);
    }
    array->data[array->size] = ptr;
    array->size++;
}

void *array_get(ArrayList *array, size_t at_index)
{
    assert(at_index < array->size);
    return array->data[at_index];
}

void * array_pop_back(ArrayList *array)
{
    assert(array);
    if (array->size == 0) {
        return NULL;
    }

    void *elem = array->data[array->size - 1];
    array->size -= 1;

    return elem;
}

void array_free(ArrayList *array)
{
    assert(array);
    assert(array->free_fn);

    for (size_t i = 0; i < array->size; i += 1) {
        array->free_fn(array->data[i]);
    }

    free(array->data);
    free(array);
}

void array_dummy_free_fn(void *ptr)
{

}

int array_is_empty(ArrayList *array)
{
    assert(array);
    return array->size == 0;
}

