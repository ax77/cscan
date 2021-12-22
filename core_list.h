#ifndef CORE_LIST_H_
#define CORE_LIST_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>
#include "core_fnptrs.h"
#include "core_mem.h"

// simple, but fast double linked list, rewritten from JDK.
// why [void*] instead of macros?
// because is really horrible use macros for complete data structures.
// yes, it safety to use typed structures, and it more clean in debugger
// after the macro being substituted.
// and you no need cast all time what you want get from link.
// and you no need to write the comment for each list, about what this list contains.
// but, and only but -> the usage of macros instead of code _MORE_ fragile
// use macros is _UNSAFE_
// you need tons of typedefs like: token_list, list_of_token_list, etc...
// and before each use you need translate this typedef in mind...
// with [void*] it more easy. you need cast, and only cast...

typedef struct linked_list LinkedList;
typedef struct list_node ListNode;

struct list_node {
    void *item;
    struct list_node *prev;
    struct list_node *next;
};

struct linked_list {
    struct list_node *first, *last;
    equal_fn equal_fn;
    size_t size;
};

LinkedList * list_new(equal_fn equal_fn);
ListNode * list_node_new(ListNode *prev, void *e, ListNode *next);
ListNode * list_get_node(LinkedList *list, size_t index);

int list_is_empty(LinkedList *list);
size_t list_size(LinkedList *list);
void list_push_front(LinkedList *list, void *e);
void list_push_back(LinkedList *list, void *e);
void * list_pop_front(LinkedList *list);
void * list_pop_back(LinkedList *list);
void * list_get(LinkedList *list, size_t at_index);
void * list_set(LinkedList *list, size_t at_index, void *element);
void * list_remove(LinkedList *list, void *o);

#endif /* CORE_LIST_H_ */
