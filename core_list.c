#include "core_list.h"

LinkedList * list_new() {
    LinkedList *list = malloc(sizeof(LinkedList));
    assert(list && "list malloc");

    list->first = list->last = NULL;
    list->size = 0;
    return list;
}

ListNode * list_node_new(ListNode *prev, void *e, ListNode *next) {
    ListNode *node = malloc(sizeof(ListNode));
    assert(node && "node malloc");
    node->prev = prev;
    node->e = e;
    node->next = next;
    return node;
}

//__attribute__((always_inline))
void list_push_front(LinkedList *list, void *e) {
    ListNode *f = list->first;
    ListNode *n = list_node_new(NULL, e, f);
    list->first = n;
    if (f == NULL) {
        list->last = n;
    } else {
        f->prev = n;
    }
    list->size++;
}

//__attribute__((always_inline))
void list_push_back(LinkedList *list, void *e) {
    ListNode *l = list->last;
    ListNode *n = list_node_new(l, e, NULL);
    list->last = n;
    if (l == NULL) {
        list->first = n;
    } else {
        l->next = n;
    }
    list->size++;
}

// TODO: free unlinked

void * list_pop_front(LinkedList *list) {
    ListNode *f = list->first;
    assert(f);

    void *elem = f->e;

    ListNode *next = f->next;
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

void * list_pop_back(LinkedList *list) {
    ListNode *l = list->last;
    assert(l);

    void *elem = l->e;

    ListNode *prev = l->prev;
    list->last = prev;
    if (prev == NULL) {
        list->first = NULL;
    } else {
        prev->next = NULL;
    }
    list->size--;

    return elem;
}

ListNode *list_get_node(LinkedList *list, size_t index) {

    assert(index < list->size);

    if (index < (list->size >> 1)) {
        ListNode *x = list->first;
        for (size_t i = 0; i < index; i++) {
            x = x->next;
        }
        return x;
    } else {
        ListNode *x = list->last;
        for (size_t i = (list->size - 1); i > index; i--) {
            x = x->prev;
        }
        return x;
    }
}

void * list_get(LinkedList *list, size_t at_index) {
    assert(at_index < list->size);
    ListNode *x = list_get_node(list, at_index);
    return x->e;
}

void * list_set(LinkedList *list, size_t at_index, void *element) {
    assert(at_index < list->size);
    ListNode *x = list_get_node(list, at_index);

    void *old = x->e;
    x->e = element;
    return old;
}

int list_is_empty(LinkedList *list) {
    return list->size == 0;
}

size_t list_size(LinkedList *list) {
    assert(list);
    return list->size;
}

