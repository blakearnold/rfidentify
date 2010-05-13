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

\code
     +---+---+   
---> | v | 0 |   
     +---+---+   
\endcode

and empty list then looks like:

\code

+---+---+   
| 0 | 0 |   
+---+---+   

\endcode

it should look like this:

\code

0

\endcode

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
typedef list list_t;

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

/*
* Higher order functions
*/
list  *list_map(list *l, void *(*fn)(void *));
list  *list_filter(list *l, int (*fn)(void *, void *), void *args);
list  *list_copy(list *l);

/**
* @param l  list *, the list to enumerate
* @param t  list *, a temporary list
*/
#define list_foreach(l, t) \
 for (t = l; ! list_is_empty(t); t = list_cdr(t)) 

/**
* @param l list *l - the list to enumerate
* @param t list *t - a temporary list
* @param type the type of the entry
* @param e       - entry of type TYPE
*/
#define list_foreach_entry(l, t, type, e) \
 for (t = l, e = (type)list_car(t); \
      ! list_is_empty(t); \
      t = list_cdr(t), e = (type)list_car(t))
   
/*
* Some destructive functions
*/
list  *list_reverse(list *l);
list  *list_append(list *l, void *v);
void  *list_remove(list *l, int index);

/*
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
