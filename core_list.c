#include "core_list.h"

LinkedList * list_new(equal_fn equal_fn)
{
    assert(equal_fn);

    LinkedList *list = cc_malloc(sizeof(LinkedList));
    list->first = list->last = NULL;
    list->size = 0;

    list->equal_fn = equal_fn;
    return list;
}

ListNode * list_node_new(ListNode *prev, void *e, ListNode *next)
{
    ListNode *node = cc_malloc(sizeof(ListNode));
    node->prev = prev;
    node->item = e;
    node->next = next;
    return node;
}

//__attribute__((always_inline))
void list_push_front(LinkedList *list, void *e)
{
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
void list_push_back(LinkedList *list, void *e)
{
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

void * list_pop_front(LinkedList *list)
{
    ListNode *f = list->first;
    assert(f);

    void *elem = f->item;

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

void * list_pop_back(LinkedList *list)
{
    ListNode *l = list->last;
    assert(l);

    void *elem = l->item;

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

ListNode *list_get_node(LinkedList *list, size_t index)
{

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

void * list_get(LinkedList *list, size_t at_index)
{
    assert(at_index < list->size);
    ListNode *x = list_get_node(list, at_index);
    return x->item;
}

void * list_set(LinkedList *list, size_t at_index, void *element)
{
    assert(at_index < list->size);
    ListNode *x = list_get_node(list, at_index);

    void *old = x->item;
    x->item = element;
    return old;
}

int list_is_empty(LinkedList *list)
{
    return list->size == 0;
}

size_t list_size(LinkedList *list)
{
    assert(list);
    return list->size;
}

//E unlink(Node<E> x) {
//    // assert x != null;
//    final E element = x.item;
//    final Node<E> next = x.next;
//    final Node<E> prev = x.prev;
//
//    if (prev == null) {
//        first = next;
//    } else {
//        prev.next = next;
//        x.prev = null;
//    }
//
//    if (next == null) {
//        last = prev;
//    } else {
//        next.prev = prev;
//        x.next = null;
//    }
//
//    x.item = null;
//    size--;
//    modCount++;
//    return element;
//}

static void *list_unlink(LinkedList *list, ListNode *x)
{
    assert(list);
    assert(x);

    void *element = x->item;
    ListNode *next = x->next;
    ListNode *prev = x->prev;

    if (prev == NULL) {
        list->first = next;
    } else {
        prev->next = next;
        x->prev = NULL;
    }

    if (next == NULL) {
        list->last = prev;
    } else {
        next->prev = prev;
        x->next = NULL;
    }

    x->item = NULL;
    list->size -= 1;

    return element;

}

//    public boolean remove(Object o) {
//        if (o == null) {
//            for (Node<E> x = first; x != null; x = x.next) {
//                if (x.item == null) {
//                    unlink(x);
//                    return true;
//                }
//            }
//        } else {
//            for (Node<E> x = first; x != null; x = x.next) {
//                if (o.equals(x.item)) {
//                    unlink(x);
//                    return true;
//                }
//            }
//        }
//        return false;
//    }

void * list_remove(LinkedList *list, void *o)
{
    assert(list);
    assert(list->equal_fn);

    if (o == NULL) {
        for (ListNode * x = list->first; x != NULL; x = x->next) {
            if (x->item == NULL) {
                return list_unlink(list, x);
            }
        }
    } else {
        for (ListNode * x = list->first; x != NULL; x = x->next) {
            if (list->equal_fn(x->item, o)) { // o.equals(x.item)
                return list_unlink(list, x);
            }
        }
    }

    return NULL;
}

