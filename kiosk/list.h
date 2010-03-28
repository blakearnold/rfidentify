#ifndef _LIST_H_
#define _LIST_H_

/**
* @author Willi Ballenthin
* @file list.h
*
* This file contains the declaration of a
* singly-linked list data structure.
*/



/**
*
standardization:

a list ends with a cell that looks like:

     +---+---+
---> | v | 0 |
     +---+---+

and empty list then looks like:

+---+---+
| 0 | 0 |
+---+---+

it should look like this:

0

but this doesnt work,
because we would need to be able to modify the pointers
but of course they are passed by value...

so this implementation needs to be careful on edge cases,
checking when there are fewer than 2 elements and whatnot

*/

typedef struct list {
  void *car;
  void *cdr;
} list;

list  *list_create();
void   list_atom_destroy(list *l);
void   list_destroy(list *l);
void   list_destroy_deep(list *l);
void  *list_car(list *l);
void  *list_cdr(list *l);
void   list_set_car(list *l, void *v);
void   list_set_cdr(list *l, void *v);
int    list_is_empty(list *l);
int    list_size(list *l);
list  *list_cons(void *v, list *l);

/**
* Higher order functions
*/
list  *list_map(list *l, void *(*fn)(void *));
list  *list_filter(list *l, int (*fn)(void *, void *), void *args);
list  *list_copy(list *l);

/**
* Some destructive functions
*/
list  *list_reverse(list *l);
list  *list_append(list *l, void *v);
void  *list_remove(list *l, int index);

/**
* Utilities
*/
void  *list_simple_free_fn(void *v);
void  *list_str_print_fn(void *);
void  *list_int_print_fn(void *);
list  *null_list();
list  *list_push(list *l, void *v);
void  *list_pop(list *l);
void  *list_nth(list *l, int i);
void **list_array(list *l);

#define ENOTFND -1
int list_index_of_int(list *haystack, int needle);
int list_index_of_str(list *haystack, const char *needle);


#endif