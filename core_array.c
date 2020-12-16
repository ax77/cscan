#include "core_array.h"
#include "core_mem.h"

ArrayList * array_new()
{
    ArrayList *rv = cc_malloc(sizeof(ArrayList));
    rv->alloc = 2;
    rv->size = 0;
    rv->data = cc_malloc(sizeof(void*) * rv->alloc);
    for (size_t i = 0; i < rv->alloc; i++) {
        rv->data[i] = NULL;
    }
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

