#include "core_list.h"

struct linked_list * list_new() {
    struct linked_list *list = malloc(sizeof(struct linked_list));
    assert(list && "list malloc");

    list->first = list->last = NULL;
    list->size = 0;
    return list;
}

struct list_node * list_node_new(struct list_node *prev, void *e,
        struct list_node *next) {
    struct list_node *node = malloc(sizeof(struct list_node));
    assert(node && "node malloc");
    node->prev = prev;
    node->e = e;
    node->next = next;
    return node;
}

//__attribute__((always_inline))
void list_push_front(struct linked_list *list, void *e) {
    struct list_node *f = list->first;
    struct list_node *n = list_node_new(NULL, e, f);
    list->first = n;
    if (f == NULL) {
        list->last = n;
    } else {
        f->prev = n;
    }
    list->size++;
}

//__attribute__((always_inline))
void list_push_back(struct linked_list *list, void *e) {
    struct list_node *l = list->last;
    struct list_node *n = list_node_new(l, e, NULL);
    list->last = n;
    if (l == NULL) {
        list->first = n;
    } else {
        l->next = n;
    }
    list->size++;
}

// TODO: free unlinked

void * list_pop_front(struct linked_list *list) {
    struct list_node *f = list->first;
    assert(f);

    void *elem = f->e;

    struct list_node *next = f->next;
    list->first = next;
    if (next == NULL) {
        list->last = NULL;
    } else {
        next->prev = NULL;
    }
    list->size--;

    return elem;
}

// TODO: free unlinked

void * list_pop_back(struct linked_list *list) {
    struct list_node *l = list->last;
    assert(l);

    void *elem = l->e;

    struct list_node *prev = l->prev;
    list->last = prev;
    if (prev == NULL) {
        list->first = NULL;
    } else {
        prev->next = NULL;
    }
    list->size--;

    return elem;
}

struct list_node *list_get_node(struct linked_list *list, size_t index) {

    assert(index < list->size);

    if (index < (list->size >> 1)) {
        struct list_node *x = list->first;
        for (size_t i = 0; i < index; i++) {
            x = x->next;
        }
        return x;
    } else {
        struct list_node *x = list->last;
        for (size_t i = (list->size - 1); i > index; i--) {
            x = x->prev;
        }
        return x;
    }
}

void * list_get(struct linked_list *list, size_t at_index) {
    assert(at_index < list->size);
    struct list_node *x = list_get_node(list, at_index);
    return x->e;
}

void * list_set(struct linked_list *list, size_t at_index, void *element) {
    assert(at_index < list->size);
    struct list_node *x = list_get_node(list, at_index);

    void *old = x->e;
    x->e = element;
    return old;
}

int list_is_empty(struct linked_list *list) {
    return list->size == 0;
}

size_t list_size(struct linked_list *list) {
    assert(list);
    return list->size;
}

