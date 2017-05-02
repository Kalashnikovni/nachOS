/// Data structures to manage LISP-like lists.
///
/// As in LISP, a list can contain any type of data structure as an item on
/// the list: thread control blocks, pending interrupts, etc.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_LIST__HH
#define NACHOS_THREADS_LIST__HH


#include "utility.hh"


/// The following class defines a “list element” -- which is used to keep
/// track of one item on a list.
///
/// Internal data structures kept public so that `List` operations can access
/// them directly.
template <class Item>
class ListElement {
public:

    // Initialize a list element.
    ListElement(Item itemPtr, int sortKey);

    ListElement *next;  ///< Next element on list, NULL if this is the last.
    int key;            ///< Priority, for a sorted list.
    Item item;          ///< Item on the list.
};

/// The following class defines a “list” -- a singly linked list of list
/// elements, each of which points to a single item on the list.
///
/// By using the `Sorted` functions, the list can be kept in sorted in
/// increasing order by `key` in `ListElement`.
template <class Item>
class List {
public:

    /// Initialize the list.
    List();

    /// De-allocate the list.
    ~List();

    /// Put item at the beginning of the list.
    void Prepend(Item item);

    /// Put item at the end of the list.
    void Append(Item item);

    /// Take item off the front of the list.
    Item Remove();

    /// Apply `func` to all elements in list.
    void Apply(void (*func)(Item));

    /// Is the list empty?
    bool IsEmpty();

    /// Routines to put/get items on/off list in order (sorted by key).

    /// Put item into list.
    void SortedInsert(Item item, int sortKey);

    /// Remove first item from list.
    Item SortedRemove(int *keyPtr);

private:

    typedef ListElement<Item> ListNode;

    ListNode *first;  ///< Head of the list, `NULL` if list is empty.
    ListNode *last;   ///< Last element of list.
};

/// Initialize a list element, so it can be added somewhere on a list.
///
/// * `anItem` is the item to be put on the list.
/// * `sortKey` is the priority of the item, if any.
template <class Item>
ListElement<Item>::ListElement(Item anItem, int sortKey)
{
     item = anItem;
     key  = sortKey;
     next = NULL;  // Assume we will put it at the end of the list.
}

/// Initialize a list, empty to start with.
///
/// Elements can now be added to the list.
template <class Item>
List<Item>::List()
{
    first = last = NULL;
}

/// Prepare a list for deallocation.
///
/// If the list still contains any `ListElement`s, de-allocate them.
/// However, note that we do *not* de-allocate the “items” on the list --
/// this module allocates and de-allocates the `ListElement`s to keep track
/// of each item, but a given item may be on multiple lists, so we cannot
/// de-allocate them here.
template <class Item>
List<Item>::~List()
{
    /// Delete all the list elements.
    while (!IsEmpty()) {
        Remove();
    }
}

// Append an “item” to the end of the list.
//
// Allocate a `ListElement` to keep track of the item.  If the list is empty,
// then this will be the only element.  Otherwise, put it at the end.
//
// * `item` is the thing to put on the list, it can be a pointer to anything.
template <class Item>
void
List<Item>::Append(Item item)
{
    ListNode *element = new ListNode(item, 0);

    if (IsEmpty()) {
        first = element;
        last = element;
    } else {  // Put it after last.
        last->next = element;
        last = element;
    }
}

/// Put an "item" on the front of the list.
///
/// Allocate a `ListElement` to keep track of the item.  If the list is
/// empty, then this will be the only element.  Otherwise, put it at the
/// beginning.
///
/// * `item` is the thing to put on the list, it can be a pointer to
///   anything.
template <class Item>
void
List<Item>::Prepend(Item item)
{
    ListNode *element = new ListNode(item, 0);

    if (IsEmpty()) {
        first = element;
        last = element;
    } else {  // Put it before first.
        element->next = first;
        first = element;
    }
}

/// Remove the first `item` from the front of the list.
///
/// Returns a pointer to removed item, `NULL` if nothing on the list.
template <class Item>
Item
List<Item>::Remove()
{
    // Same as `SortedRemove`, but ignore the key.
    return SortedRemove(NULL);
}

/// Apply a function to each item on the list, by walking through the list,
/// one element at a time.
///
/// * `func` is the procedure to apply to each element of the list.
template <class Item>
void
List<Item>::Apply(void (*func)(Item))
{
    for (ListNode *ptr = first; ptr != NULL; ptr = ptr->next) {
       func(ptr->item);
    }
}

/// Returns true if the list is empty (has no items).
template <class Item>
bool
List<Item>::IsEmpty()
{
    return first == NULL;
}

/// Insert an `item` into a list, so that the list elements are sorted in
/// increasing order by `sortKey`.
///
/// Allocate a `ListElement` to keep track of the item.  If the list is
/// empty, then this will be the only element.  Otherwise, walk through the
/// list, one element at a time, to find where the new item should be placed.
///
/// * `item` is the thing to put on the list, it can be a pointer to
///   anything.
/// * `sortKey` is the priority of the item.
template <class Item>
void
List<Item>::SortedInsert(Item item, int sortKey)
{
    ListNode *element = new ListNode(item, sortKey);
    ListNode *ptr;  // Keep track.

    if (IsEmpty()) {  // If list is empty, put.
        first = element;
        last = element;
    } else if (sortKey < first->key) {
        // Item goes on front of list.
        element->next = first;
        first = element;
    } else {  // Look for first elt in list bigger than item.
        for (ptr = first; ptr->next != NULL; ptr = ptr->next) {
            if (sortKey < ptr->next->key) {
                element->next = ptr->next;
                ptr->next = element;
                return;
            }
        }
        last->next = element;  // Item goes at end of list.
        last = element;
    }
}

/// Remove the first “item” from the front of a sorted list.
///
/// Returns pointer to removed item, `NULL` if nothing on the list.
///
/// Sets `*keyPtr` to the priority value of the removed item (this is needed
/// by `interrupt.cc`, for instance).
///
/// * `keyPtr` is a pointer to the location in which to store the priority of
///   the removed item.
template <class Item>
Item
List<Item>::SortedRemove(int *keyPtr)
{
    ListNode *element = first;

    if (IsEmpty())
        return Item();

    Item thing = first->item;
    if (first == last) {  // List had one item, now has none.
        first = NULL;
        last = NULL;
    } else {
        first = element->next;
    }
    if (keyPtr != NULL)
        *keyPtr = element->key;
    delete element;
    return thing;
}


#endif
