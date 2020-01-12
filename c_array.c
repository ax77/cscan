#include "c_array.h"
#include "core_mem.h"

ArrayList *ArrayList_new()
{
    ArrayList *r = cc_malloc(sizeof(ArrayList));
    r->len = 0;
    r->alloc = 8;
    r->table = cc_malloc(r->alloc * sizeof(ArrayList *));
    return r;
}

void ArrayList_add(ArrayList *list, void *what)
{
    assert(list);
    assert(what);
    if (list->len >= list->alloc) {
        list->alloc *= 2;
        list->table = cc_realloc(list->table, list->alloc * sizeof(ArrayList*));
    }
    list->table[list->len] = what;
    list->len++;
}

void *ArrayList_pop_back(ArrayList *list)
{
    assert(list);
    assert(list->len > 0);
    void * ptr = list->table[list->len - 1];
    list->len--;
    return ptr;
}

void *ArrayList_get(ArrayList *list, size_t index) {
    assert(list);
    assert(index < list->len);
    return list->table[index];
}


















