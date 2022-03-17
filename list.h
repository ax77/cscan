/*
 * list.h
 *
 *  Created on: 16 мар. 2022 г.
 *      Author: Alexey
 */

#ifndef LIST_H_
#define LIST_H_

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct List List;
typedef struct Node Node;

struct Node {
    struct Node *prev;
    struct Node *next;
    void *item;
};

struct List {
    struct Node *first;
    struct Node *last;
    size_t size;
};

List *list_new();
void *list_get_first(List *list);
void *list_get_last(List *list);

void list_add_first(List *list, void *e);
void list_add_last(List *list, void *e);
void *list_remove_first(List *list);
void *list_remove_last(List *list);

void *list_get(List *list, size_t index);
void *list_set(List *list, size_t index, void *element);

void list_add(List *list, size_t index, void *element);
void *list_remove(List *list, size_t index);
ptrdiff_t list_index_of(List *list, void *o, int (*equals)(void*, void*));
void **list_to_array(List *list);

void list_test0();
void list_test1();
void list_test2();
void list_test3();
void list_test4();
void list_test5();
void list_test6();

#endif /* LIST_H_ */
