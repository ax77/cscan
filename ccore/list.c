#include "list.h"

// Test routine

static void *xmalloc(size_t size)
{
    assert(size > 0);
    assert(size<INT_MAX);

    void *ptr = NULL;
    ptr = calloc(1u, size);
    if (ptr == NULL) {
        ptr = calloc(1u, size);
        if (ptr == NULL) {
            ptr = calloc(1u, size);
        }
    }

    assert(ptr);
    return ptr;
}

#define assert_true(expr) do {\
  if( !(expr) ) {\
    fprintf(stderr, "test fail: (%s:%s():%d) : [%s]\n" \
    , __FILE__, __func__, __LINE__, #expr);\
    exit(128);\
  }\
} while(0)

static char *str(char *from)
{
    assert(from);
    size_t len = strlen(from);
    char *r = xmalloc(len + 1);
    strcpy(r, from);
    r[len] = '\0';
    return r;
}

static int streq(char *s1, char *s2)
{
    assert(s1);
    assert(s2);
    return strcmp(s1, s2) == 0;
}

/// private
static int is_element_index(List *list, size_t index);
static int is_position_index(List *list, size_t index);
static char *out_of_bounds_msg(List *list, size_t index);
static void exit_with_msg(char *msg);
static void check_element_index(List *list, size_t index);
static void check_position_index(List *list, size_t index);

static Node *node_new(Node *prev, void *element, Node *next);
static Node *node(List *list, size_t index);

static void list_link_first(List *list, void *e);
static void list_link_last(List *list, void *e);
static void list_link_before(List *list, void *e, Node *succ);
static void *list_unlink_first(List *list, Node *f);
static void *list_unlink_last(List *list, Node *l);
static void *list_unlink(List *list, Node *x);

List *list_new()
{
    struct List *r = xmalloc(sizeof(struct List));
    r->first = r->last = NULL;
    r->size = 0;
    return r;
}

/// private static class Node<E> {
///     E item;
///     Node<E> next;
///     Node<E> prev;
///
///     Node(Node<E> prev, E element, Node<E> next) {
///         this.item = element;
///         this.next = next;
///         this.prev = prev;
///     }
/// }

Node *node_new(Node *prev, void *element, Node *next)
{
    struct Node *r = xmalloc(sizeof(struct Node));
    r->prev = prev;
    r->item = element;
    r->next = next;
    return r;
}

/// /**
///  * Links e as first element.
///  */
/// private void linkFirst(E e) {
///     final Node<E> f = first;
///     final Node<E> newNode = new Node<>(null, e, f);
///     first = newNode;
///     if (f == null)
///         last = newNode;
///     else
///         f.prev = newNode;
///     size++;
///     modCount++;
/// }

static void list_link_first(List *list, void *e)
{
    assert(list);
    Node *f = list->first;
    Node *n = node_new(NULL, e, f);
    list->first = n;
    if (f == NULL) {
        list->last = n;
    } else {
        f->prev = n;
    }
    list->size++;
}

/// /**
///  * Links e as last element.
///  */
/// void linkLast(E e) {
///     final Node<E> l = last;
///     final Node<E> newNode = new Node<>(l, e, null);
///     last = newNode;
///     if (l == null)
///         first = newNode;
///     else
///         l.next = newNode;
///     size++;
///     modCount++;
/// }

static void list_link_last(List *list, void *e)
{
    assert(list);
    Node *l = list->last;
    Node *n = node_new(l, e, NULL);
    list->last = n;
    if (l == NULL) {
        list->first = n;
    } else {
        l->next = n;
    }
    list->size++;
}

/// /**
///  * Inserts element e before non-null Node succ.
///  */
/// void linkBefore(E e, Node<E> succ) {
///     // assert succ != null;
///     final Node<E> pred = succ.prev;
///     final Node<E> newNode = new Node<>(pred, e, succ);
///     succ.prev = newNode;
///     if (pred == null)
///         first = newNode;
///     else
///         pred.next = newNode;
///     size++;
///     modCount++;
/// }

static void list_link_before(List *list, void *e, Node *succ)
{
    assert(list);
    assert(succ);

    Node *pred = succ->prev;
    Node *n = node_new(pred, e, succ);
    succ->prev = n;
    if (pred == NULL) {
        list->first = n;
    } else {
        pred->next = n;
    }
    list->size++;
}

/// /**
///  * Unlinks non-null first node f.
///  */
/// private E unlinkFirst(Node<E> f) {
///     // assert f == first && f != null;
///     final E element = f.item;
///     final Node<E> next = f.next;
///     f.item = null;
///     f.next = null; // help GC
///     first = next;
///     if (next == null)
///         last = null;
///     else
///         next.prev = null;
///     size--;
///     modCount++;
///     return element;
/// }

static void *list_unlink_first(List *list, Node *f)
{
    assert(list);
    assert(f);
    assert(f == list->first);

    void *element = f->item;
    Node *next = f->next;

    list->first = next;
    if (next == NULL) {
        list->last = NULL;
    } else {
        next->prev = NULL;
    }

    list->size--;
    return element;
}

/// /**
///  * Unlinks non-null last node l.
///  */
/// private E unlinkLast(Node<E> l) {
///     // assert l == last && l != null;
///     final E element = l.item;
///     final Node<E> prev = l.prev;
///     l.item = null;
///     l.prev = null; // help GC
///     last = prev;
///     if (prev == null)
///         first = null;
///     else
///         prev.next = null;
///     size--;
///     modCount++;
///     return element;
/// }

static void *list_unlink_last(List *list, Node *l)
{
    assert(list);
    assert(l);
    assert(l == list->last);

    void *element = l->item;
    Node *prev = l->prev;

    list->last = prev;
    if (prev == NULL) {
        list->first = NULL;
    } else {
        prev->next = NULL;
    }

    list->size--;
    return element;
}

/// /**
///  * Unlinks non-null node x.
///  */
/// E unlink(Node<E> x) {
///     // assert x != null;
///     final E element = x.item;
///     final Node<E> next = x.next;
///     final Node<E> prev = x.prev;
///
///     if (prev == null) {
///         first = next;
///     } else {
///         prev.next = next;
///         x.prev = null;
///     }
///
///     if (next == null) {
///         last = prev;
///     } else {
///         next.prev = prev;
///         x.next = null;
///     }
///
///     x.item = null;
///     size--;
///     modCount++;
///     return element;
/// }

static void *list_unlink(List *list, Node *x)
{
    assert(list);
    assert(x);

    void *element = x->item;
    Node *next = x->next;
    Node *prev = x->prev;

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

    list->size--;
    return element;
}

/// /**
///  * Returns the first element in this list.
///  *
///  * @return the first element in this list
///  * @throws NoSuchElementException if this list is empty
///  */
/// public E getFirst() {
///     final Node<E> f = first;
///     if (f == null)
///         throw new NoSuchElementException();
///     return f.item;
/// }

void *list_get_first(List *list)
{
    assert(list);
    Node *f = list->first;
    assert(f);
    return f->item;
}

/// /**
///  * Returns the last element in this list.
///  *
///  * @return the last element in this list
///  * @throws NoSuchElementException if this list is empty
///  */
/// public E getLast() {
///     final Node<E> l = last;
///     if (l == null)
///         throw new NoSuchElementException();
///     return l.item;
/// }

void *list_get_last(List *list)
{
    assert(list);
    Node *l = list->last;
    assert(l);
    return l->item;
}

/// /**
///  * Removes and returns the first element from this list.
///  *
///  * @return the first element from this list
///  * @throws NoSuchElementException if this list is empty
///  */
/// public E removeFirst() {
///     final Node<E> f = first;
///     if (f == null)
///         throw new NoSuchElementException();
///     return unlinkFirst(f);
/// }

void *list_remove_first(List *list)
{
    assert(list);
    Node *f = list->first;
    assert(f);
    return list_unlink_first(list, f);
}

/// /**
///  * Removes and returns the last element from this list.
///  *
///  * @return the last element from this list
///  * @throws NoSuchElementException if this list is empty
///  */
/// public E removeLast() {
///     final Node<E> l = last;
///     if (l == null)
///         throw new NoSuchElementException();
///     return unlinkLast(l);
/// }

void *list_remove_last(List *list)
{
    assert(list);
    Node *l = list->last;
    assert(l);
    return list_unlink_last(list, l);
}

/// /**
///  * Inserts the specified element at the beginning of this list.
///  *
///  * @param e the element to add
///  */
/// public void addFirst(E e) {
///     linkFirst(e);
/// }

void list_add_first(List *list, void *e)
{
    assert(list);
    list_link_first(list, e);
}

/// /**
///  * Appends the specified element to the end of this list.
///  *
///  * <p>This method is equivalent to {@link #add}.
///  *
///  * @param e the element to add
///  */
/// public void addLast(E e) {
///     linkLast(e);
/// }

void list_add_last(List *list, void *e)
{
    assert(list);
    list_link_last(list, e);
}

/// /**
///  * Returns {@code true} if this list contains the specified element.
///  * More formally, returns {@code true} if and only if this list contains
///  * at least one element {@code e} such that
///  * {@code Objects.equals(o, e)}.
///  *
///  * @param o element whose presence in this list is to be tested
///  * @return {@code true} if this list contains the specified element
///  */
/// public boolean contains(Object o) {
///     return indexOf(o) >= 0;
/// }
///
/// /**
///  * Returns the number of elements in this list.
///  *
///  * @return the number of elements in this list
///  */
/// public int size() {
///     return size;
/// }
///
/// /**
///  * Appends the specified element to the end of this list.
///  *
///  * <p>This method is equivalent to {@link #addLast}.
///  *
///  * @param e element to be appended to this list
///  * @return {@code true} (as specified by {@link Collection#add})
///  */
/// public boolean add(E e) {
///     linkLast(e);
///     return true;
/// }
///
/// /**
///  * Removes the first occurrence of the specified element from this list,
///  * if it is present.  If this list does not contain the element, it is
///  * unchanged.  More formally, removes the element with the lowest index
///  * {@code i} such that
///  * {@code Objects.equals(o, get(i))}
///  * (if such an element exists).  Returns {@code true} if this list
///  * contained the specified element (or equivalently, if this list
///  * changed as a result of the call).
///  *
///  * @param o element to be removed from this list, if present
///  * @return {@code true} if this list contained the specified element
///  */
/// public boolean remove(Object o) {
///     if (o == null) {
///         for (Node<E> x = first; x != null; x = x.next) {
///             if (x.item == null) {
///                 unlink(x);
///                 return true;
///             }
///         }
///     } else {
///         for (Node<E> x = first; x != null; x = x.next) {
///             if (o.equals(x.item)) {
///                 unlink(x);
///                 return true;
///             }
///         }
///     }
///     return false;
/// }
///
/// /**
///  * Removes all of the elements from this list.
///  * The list will be empty after this call returns.
///  */
/// public void clear() {
///     // Clearing all of the links between nodes is "unnecessary", but:
///     // - helps a generational GC if the discarded nodes inhabit
///     //   more than one generation
///     // - is sure to free memory even if there is a reachable Iterator
///     for (Node<E> x = first; x != null; ) {
///         Node<E> next = x.next;
///         x.item = null;
///         x.next = null;
///         x.prev = null;
///         x = next;
///     }
///     first = last = null;
///     size = 0;
///     modCount++;
/// }
///

/// // Positional Access Operations
///
/// /**
///  * Returns the element at the specified position in this list.
///  *
///  * @param index index of the element to return
///  * @return the element at the specified position in this list
///  * @throws IndexOutOfBoundsException {@inheritDoc}
///  */
/// public E get(int index) {
///     checkElementIndex(index);
///     return node(index).item;
/// }

void *list_get(List *list, size_t index)
{
    assert(list);
    check_element_index(list, index);
    return node(list, index)->item;
}

/// /**
///  * Replaces the element at the specified position in this list with the
///  * specified element.
///  *
///  * @param index index of the element to replace
///  * @param element element to be stored at the specified position
///  * @return the element previously at the specified position
///  * @throws IndexOutOfBoundsException {@inheritDoc}
///  */
/// public E set(int index, E element) {
///     checkElementIndex(index);
///     Node<E> x = node(index);
///     E oldVal = x.item;
///     x.item = element;
///     return oldVal;
/// }

void *list_set(List *list, size_t index, void *element)
{
    assert(list);
    check_element_index(list, index);
    Node *x = node(list, index);
    void *old = x->item;
    x->item = element;
    return old;
}

/// /**
///  * Inserts the specified element at the specified position in this list.
///  * Shifts the element currently at that position (if any) and any
///  * subsequent elements to the right (adds one to their indices).
///  *
///  * @param index index at which the specified element is to be inserted
///  * @param element element to be inserted
///  * @throws IndexOutOfBoundsException {@inheritDoc}
///  */
/// public void add(int index, E element) {
///     checkPositionIndex(index);
///
///     if (index == size)
///         linkLast(element);
///     else
///         linkBefore(element, node(index));
/// }

void list_add(List *list, size_t index, void *element)
{
    assert(list);
    check_position_index(list, index);

    if (index == list->size) {
        list_link_last(list, element);
    } else {
        list_link_before(list, element, node(list, index));
    }
}

/// /**
///  * Removes the element at the specified position in this list.  Shifts any
///  * subsequent elements to the left (subtracts one from their indices).
///  * Returns the element that was removed from the list.
///  *
///  * @param index the index of the element to be removed
///  * @return the element previously at the specified position
///  * @throws IndexOutOfBoundsException {@inheritDoc}
///  */
/// public E remove(int index) {
///     checkElementIndex(index);
///     return unlink(node(index));
/// }

void *list_remove(List *list, size_t index)
{
    assert(list);
    check_element_index(list, index);
    return list_unlink(list, node(list, index));
}

/// /**
///  * Tells if the argument is the index of an existing element.
///  */
/// private boolean isElementIndex(int index) {
///     return index >= 0 && index < size;
/// }

static int is_element_index(List *list, size_t index)
{
    assert(list);
    return index < list->size;
}

/// /**
///  * Tells if the argument is the index of a valid position for an
///  * iterator or an add operation.
///  */
/// private boolean isPositionIndex(int index) {
///     return index >= 0 && index <= size;
/// }

static int is_position_index(List *list, size_t index)
{
    assert(list);
    return index <= list->size;
}

/// /**
///  * Constructs an IndexOutOfBoundsException detail message.
///  * Of the many possible refactorings of the error handling code,
///  * this "outlining" performs best with both server and client VMs.
///  */
/// private String outOfBoundsMsg(int index) {
///     return "Index: "+index+", Size: "+size;
/// }

static char *out_of_bounds_msg(List *list, size_t index)
{
    assert(list);
    static char buffer[256];
    sprintf(buffer, "index=%lu, size=%lu", index, list->size);
    return buffer;
}

static void exit_with_msg(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(3);
}

/// private void checkElementIndex(int index) {
///     if (!isElementIndex(index))
///         throw new IndexOutOfBoundsException(outOfBoundsMsg(index));
/// }

static void check_element_index(List *list, size_t index)
{
    if (!is_element_index(list, index)) {
        exit_with_msg(out_of_bounds_msg(list, index));
    }
}

/// private void checkPositionIndex(int index) {
///     if (!isPositionIndex(index))
///         throw new IndexOutOfBoundsException(outOfBoundsMsg(index));
/// }

static void check_position_index(List *list, size_t index)
{
    if (!is_position_index(list, index)) {
        exit_with_msg(out_of_bounds_msg(list, index));
    }
}

/// /**
///  * Returns the (non-null) Node at the specified element index.
///  */
/// Node<E> node(int index) {
///     // assert isElementIndex(index);
///
///     if (index < (size >> 1)) {
///         Node<E> x = first;
///         for (int i = 0; i < index; i++)
///             x = x.next;
///         return x;
///     } else {
///         Node<E> x = last;
///         for (int i = size - 1; i > index; i--)
///             x = x.prev;
///         return x;
///     }
/// }

static Node *node(List *list, size_t index)
{
    assert(list);
    check_element_index(list, index);

    if (index < (list->size >> 1)) {
        Node *x = list->first;
        for (size_t i = 0; i < index; i++) {
            x = x->next;
        }
        return x;
    } else {
        Node *x = list->last;
        for (size_t i = (list->size - 1); i > index; i--) {
            x = x->prev;
        }
        return x;
    }
}

/// // Search Operations
///
/// /**
///  * Returns the index of the first occurrence of the specified element
///  * in this list, or -1 if this list does not contain the element.
///  * More formally, returns the lowest index {@code i} such that
///  * {@code Objects.equals(o, get(i))},
///  * or -1 if there is no such index.
///  *
///  * @param o element to search for
///  * @return the index of the first occurrence of the specified element in
///  *         this list, or -1 if this list does not contain the element
///  */
/// public int indexOf(Object o) {
///     int index = 0;
///     if (o == null) {
///         for (Node<E> x = first; x != null; x = x.next) {
///             if (x.item == null)
///                 return index;
///             index++;
///         }
///     } else {
///         for (Node<E> x = first; x != null; x = x.next) {
///             if (o.equals(x.item))
///                 return index;
///             index++;
///         }
///     }
///     return -1;
/// }

ptrdiff_t list_index_of(List *list, void *o, int (*equals)(void*, void*))
{
    assert(list);

    ptrdiff_t index = 0;
    if (o == NULL) {
        for (Node *x = list->first; x != NULL; x = x->next) {
            if (x->item == NULL) {
                return index;
            }
            index++;
        }
    } else {
        for (Node *x = list->first; x != NULL; x = x->next) {
            if (equals(o, x->item)) {
                return index;
            }
            index++;
        }
    }
    return -1;
}

/// /**
///  * Retrieves, but does not remove, the first element of this list,
///  * or returns {@code null} if this list is empty.
///  *
///  * @return the first element of this list, or {@code null}
///  *         if this list is empty
///  * @since 1.6
///  */
/// public E peekFirst() {
///     final Node<E> f = first;
///     return (f == null) ? null : f.item;
///  }
///
/// /**
///  * Retrieves, but does not remove, the last element of this list,
///  * or returns {@code null} if this list is empty.
///  *
///  * @return the last element of this list, or {@code null}
///  *         if this list is empty
///  * @since 1.6
///  */
/// public E peekLast() {
///     final Node<E> l = last;
///     return (l == null) ? null : l.item;
/// }
///
/// /**
///  * Pushes an element onto the stack represented by this list.  In other
///  * words, inserts the element at the front of this list.
///  *
///  * <p>This method is equivalent to {@link #addFirst}.
///  *
///  * @param e the element to push
///  * @since 1.6
///  */
/// public void push(E e) {
///     addFirst(e);
/// }
///
/// /**
///  * Pops an element from the stack represented by this list.  In other
///  * words, removes and returns the first element of this list.
///  *
///  * <p>This method is equivalent to {@link #removeFirst()}.
///  *
///  * @return the element at the front of this list (which is the top
///  *         of the stack represented by this list)
///  * @throws NoSuchElementException if this list is empty
///  * @since 1.6
///  */
/// public E pop() {
///     return removeFirst();
/// }

/// We're able to parse 'something' in a list,
/// and then store the result in a data-structure we want.
///
/// Example:
/// struct function {
///     struct parameter **params;
///     size_t params_cnt;
/// };
/// When we parse the source file, we use our List, and
/// then then invoke 'to_array()'
/// like this:
///
/// List *list = parse_params();
/// struct parameter **result = (struct parameter **) to_array(list);
/// struct function *varname = function_new();
/// varname->params = result;
/// varname->params_cnt = list->size;
/// list_destroy(list);
///
void **list_to_array(List *list)
{
    assert(list);
    size_t size = list->size;
    if (size == 0) {
        size = 1;
    }
    void **arr = xmalloc(size * sizeof(void*));
    size_t index = 0;

    for (Node *x = list->first; x != NULL; x = x->next) {
        arr[index] = x->item;
        index++;
    }

    return arr;
}

void list_test0()
{
    List *h = list_new();

    // 1
    list_link_first(h, str("1"));
    assert_true(1 == h->size);
    assert_true(streq("1", h->first->item));

    // 1 2
    list_link_last(h, str("2"));
    assert_true(2 == h->size);
    assert_true(streq("1", h->first->item));
    assert_true(streq("2", h->last->item));

    // . 1 2
    list_link_first(h, str("."));
    assert_true(3 == h->size);
    assert_true(streq(".", h->first->item));
    assert_true(streq("1", h->first->next->item));

    // . 1 2 @
    list_link_last(h, str("@"));
    assert_true(4 == h->size);
    assert_true(streq(".", h->first->item));
    assert_true(streq("1", h->first->next->item));
    assert_true(streq("2", h->first->next->next->item));
    assert_true(streq("@", h->first->next->next->next->item));
    assert_true(NULL == h->first->next->next->next->next);

    assert_true(streq("@", h->last->item));
    assert_true(streq("2", h->last->prev->item));
    assert_true(streq("1", h->last->prev->prev->item));
    assert_true(streq(".", h->last->prev->prev->prev->item));
    assert_true(NULL == h->last->prev->prev->prev->prev);
}

void list_test1()
{
    List *h = list_new();

    list_add(h, 0, str("1"));
    assert_true(1 == h->size);
    assert_true(streq("1", h->first->item));

    list_add(h, 0, str("0"));
    assert_true(2 == h->size);
    assert_true(streq("0", h->first->item));
    assert_true(streq("1", h->first->next->item));
}

void list_test2()
{
    List *h = list_new();
    list_link_first(h, str("1"));
    assert_true(1 == h->size);
    assert_true(streq("1", h->first->item));

    char *s = list_remove_first(h);
    assert_true(streq("1", s));
    assert_true(0 == h->size);
}

void list_test3()
{
    List *h = list_new();
    list_link_first(h, str("2"));
    list_link_first(h, str("1"));
    assert_true(2 == h->size);
    assert_true(streq("1", h->first->item));

    assert_true(streq("1", list_remove_first(h)));
    assert_true(streq("2", list_remove_first(h)));
    assert_true(0 == h->size);
}

void list_test4()
{
    List *h = list_new();
    list_link_first(h, str("2"));
    list_link_first(h, str("1"));
    assert_true(2 == h->size);
    assert_true(streq("1", h->first->item));

    assert_true(streq("2", list_remove_last(h)));
    assert_true(streq("1", list_remove_last(h)));
    assert_true(0 == h->size);
}

void list_test5()
{
    List *h = list_new();
    list_add_last(h, str("31"));
    list_add_last(h, str("32"));

    assert_true(streq("31", h->first->item));
    assert_true(streq("32", h->last->item));

    void *f = (int (*)(void*, void*)) streq;
    assert_true(0 == list_index_of(h, "31", f));
    assert_true(1 == list_index_of(h, "32", f));
    assert_true(-1 == list_index_of(h, "33", f));
}

void list_test6()
{
    List *h = list_new();
    list_add_last(h, str("31"));
    list_add_last(h, str("32"));
    list_add_last(h, str("33"));
    list_add_last(h, str("34"));
    list_add_last(h, str("35"));

    char **arr = (char**) list_to_array(h);
    assert_true(streq("31", arr[0]));
    assert_true(streq("32", arr[1]));
    assert_true(streq("33", arr[2]));
    assert_true(streq("34", arr[3]));
    assert_true(streq("35", arr[4]));
}
