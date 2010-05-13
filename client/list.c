/**
 * @author Willi Ballenthin
 * @file list.c
 *
 * This file contains the definition of a
 * singly-linked list data structure.
 * It was developed as I learned and explored
 * LISP.
 * The influence of LISP is clear in the API.
 */

#include "list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int TRUE = 1;

/**
 *	@return A newly allocated list, or NULL on failure
 */
list *list_create() {
  list *s;
  if ( ! ( s = (list *)malloc(1 * sizeof(list))))
    goto emem;

  s->car = NULL;
  s->cdr = NULL;
  return s;

 emem:
  free(s);
  return NULL;
}

/**
 * @param l a list.
 * @return The car of the list.
 */
void *list_car(list *l) {
  if ( !l )
    return NULL;
  return l->car;
}
/**
 * @param a list
 * @return The cdr of the list.
 */
void *list_cdr(list *l) {
  if ( !l )
    return NULL;
  return l->cdr;
}

/**
 * Sets the car of a list.
 * @param l list.
 * @param v the value.
 */
void list_set_car(list *l, void *v) {
  if ( !l ) {
    printf("Warning: Set Car: null list.\n.");
    return;
  }
  l->car = v;
  return;
}

/**
 * Sets the cdr of a list.
 * @param l list.
 * @param v the value.
 */
void list_set_cdr(list *l, void *v) {
  if ( !l ) {
    printf("Warning: Set Cdr: null list.\n.");
    return;
  }
  l->cdr = v;
  return;
}

/** 
 * @param l a list.
 * @return true if the parameter is null.
 */
int list_is_null(list *l) {
  return l == NULL;
}

/**
 * @param l a list.
 * @return true if the parameter is empty
 */
int list_is_empty(list *l) {
  return list_is_null(l) || (list_car(l) == NULL &&
			     list_cdr(l) == NULL);
}

/**
 * @return size of list.
 */
int list_size(list *l) {
  int count;
  list *p;

  if (list_is_empty(l)) {
    return 0;
  }

  if ( list_car(l) &&
       !list_cdr(l)) {
    return 1;
  }

  count = 1;
  p     = l;
  while (list_cdr(p)) {
    p = list_cdr(p);
    count++;
  }
  return count;
}

/**
 * @return a list formed of the given value cons-ed with
 * 	the given list. Allocates memory. Returns NULL on
 *	failure.
 */
list *list_cons(void *v, list *l) {
  list *k;

  if (list_is_null(l)) {
    if ( ! (k = list_create()))
      goto emem;
    list_set_car(k, v);
    return k;
  }
  if (list_is_empty(l)) {
    list_set_car(l, v);
    return l;
  }

  if ( ! (k = list_create()))
    goto emem;
  list_set_car(k, v);
  list_set_cdr(k, l);
  return k;

 emem:
  free(k);
  return NULL;
}

/**
 * destructively modifies the current list.
 * note, you must assign the result back in place, for
 * this method is unable to modify in place,
 * at the current time.
 * @return the list reversed. pointer to new head.
 */
list *list_reverse(list *l) {
  list *p, *q, *r;
  
  if (list_size(l) <= 1) {
    return l;
  }

  // special cases, because we
  // use pointers to 3 nodes otherwise
  if (list_size(l) == 2) {
    p = l;
    q = list_cdr(p);

    if (list_is_empty(q)) {
      return p;
    }
    else {
      list_set_cdr(p, NULL);
      list_set_cdr(q, p);
      return q;
    }
  }
  if (list_size(l) == 3) {
    p = l;
    q = list_cdr(l);
    r = list_cdr(q);

    list_set_cdr(p, NULL);
    list_set_cdr(q, p);
    list_set_cdr(r, q);
    return r;
  }

  p = l;
  q = list_cdr(l);
  r = list_cdr(q);
  
  list_set_cdr(p, NULL);
  list_set_cdr(q, p);
  p = q;
  q = r;
  r = list_cdr(r);

  while ( list_cdr(r) ) {
    list_set_cdr(q, p);
    p = q;
    q = r;
    r = list_cdr(r);
  }
  
  list_set_cdr(q, p);
  list_set_cdr(r, q);
  return r;
}

/*****************************************************
 *
 *	DEALLOCATION FUNCTIONS
 *
 *****************************************************/

/**
 * inverse of list_create()
 * does not free contents
 */
void list_atom_destroy(list *l) {
  // its ok to free NULL
  free(l);
  return;
}

/**
 * list_atom_destroy() chained together across
 * an entire list.
 * currently recursive, so probs not a good function
 * for large lists...
 * although i heard GCC can optimize tail recursion,
 * so this may be fine
 */
void list_destroy(list *l) {
  list *next;
  if (list_is_null(l)) {
    return;
  }
  if (! list_cdr(l)) {
    list_atom_destroy(l);
    return;
  }

  // so this is recursive, which is bad for huge lists...
  next = list_cdr(l);
  list_atom_destroy(l);

  return list_destroy(next);
}

/**
 * like list_destroy() but calls free on each
 * car.
 */
void list_destroy_deep(list *l) {
  if (list_is_null(l)) {
    return;
  }
  if (list_is_empty(l)) {
    list_atom_destroy(l);
    return;
  }

  // so this is recursive, which is bad for huge lists...
  list *n = list_cdr(l);
  free(list_car(l));
  list_atom_destroy(l);
  return list_destroy_deep(n);
}

/**********************************************************
 *
 *	HIGHER ORDER FUNCTIONS
 *
 **********************************************************/


/**
 * Maps a function across all elements of a list
 * @return a new list of the results, or NULL on failure
 */ 
list *list_map(list *l, void *(*fn)(void *)) {
  list *acc;
  
  if (list_is_empty(l)) {
    return null_list(); // may return NULL
  }

  if ( ! (acc = null_list()))
    goto emem;

  while ( ! list_is_empty(l)) {
    void *new_value = fn(list_car(l));
    if ( ! (acc = list_cons(new_value, acc)))
      goto emem;
    l = list_cdr(l);
  }
  return acc;

 emem:
  // if memory fails, it is troubling
  // since the user may allocate (or not) within fn.
  // we return NULL and leak memory until I can think of
  // something better
  return NULL;
}

/**
 * Given a filtering function,
 * @param l The list to filter.
 * @param fn A function to apply to each element in the list. If the
 *   application is TRUE, then the element is retained, else it is not
 *   included in the new list.
 * @param args Arguments to be passed to the filtering function.
 * @return a new list consisting of elements of the old list that pass the test
 * 	or NULL on failure.
 */
list *list_filter(list *l, int (*fn)(void *, void *), void *args) {
  list *acc = NULL, *old_acc = NULL;
  
  if (list_is_empty(l)) {
    return null_list(); // may return NULL
  }

  if ( ! (acc = null_list()))
    goto emem;
  
  old_acc = acc;
  while ( ! list_is_empty(l)) {
    if (fn(list_car(l), args)) {
      old_acc = acc;
      if ( ! (acc = list_cons(list_car(l), acc)))
	goto emem;
    }
    l = list_cdr(l);
  }
  return acc;

 emem:
  free(acc);
  list_destroy(old_acc);
  return NULL;
}

/************************************************************
 *
 *	UTILITY FUNCTIONS
 *
 ************************************************************/

/**
 * the following 3 functions conform to the function
 * signatures accepted by list_map and list_filter
 */


void  *list_simple_free_fn(void *v) {
  free(v);
  return &TRUE;
}

void *list_str_print_fn(void *v) {
  printf("[%s]\n", (char *)v);

  return &TRUE;
}

void *list_int_print_fn(void *v) {
  printf("[%d]\n", *((int *)v));

  return &TRUE;
}

/**
 * @return a new empty list, or NULL on failure.
 */
list *null_list() {
  return list_create();
}

/**
 * @return the value at the nth position of the list.
 * 	NULL on failure, however accessing past the end of
 *	the list is undefined.
 */
void *list_nth(list *l, int i) {
  if (i == 0) {
    if ( ! l)
      return NULL;
    return list_car(l);
  }
  // GCC optimizes tail recursion.
  return list_nth(list_cdr(l), --i);
}

/**
 * @return a new array, filled with the contents of the provided list
 * the list is NULL terminated, but since the values of the array may be
 * NULL, this should not be used to determine the length of the array.
 * rather, list_size() should be used. NULL on failure or empty list.
 */
void **list_array(list *l) {

  void **ar;
  void **ap;
  list *p;

  if (list_size(l) == 0) {
    if ( ! (ar = (void **)malloc(sizeof(void *) * 1)))
      goto emem;
    *ar = NULL;
    return ar;
  }
  
  if ( ! (ar = (void **)malloc(sizeof(void *) * (list_size(l) + 1))))
    goto emem;
  ap = ar;
  p = l;

  while ( ! list_is_empty(p)) {
    *ap = list_car(p);
    ap++;
    p = list_cdr(p);
  }
  *ap = NULL;

  return ar;

 emem:
  free(ar);
  return NULL;
}

/**
 * @return index of first eleemnt encountered matching needle,
 * 	ENOTFND otherwise.
 */
int list_index_of_int(list *haystack, int needle) {
  int count = 0;
  if (list_is_empty(haystack))
    return ENOTFND;

  while( ! list_is_empty(haystack)) {
    if (*(int *)list_car(haystack) == needle) {
      return count;
    }
    else {
      count++;
      haystack = list_cdr(haystack);
    }
  }
  return ENOTFND;
}

/**
 * @return index of first eleemnt encountered matching needle,
 * 	ENOTFND otherwise.
 */
int list_index_of_str(list *haystack, const char *needle) {
  int count = 0;
  if (list_is_empty(haystack))
    return ENOTFND;

  while( ! list_is_empty(haystack)) {
    if (strcmp(list_car(haystack), needle) == 0) {
      return count;
    }
    else {
      count++;
      haystack = list_cdr(haystack);
    }
  }
  return ENOTFND;
}

/********************************************************
 *
 *	DESTRUCTIVE LIST FUNCTIONS
 *
 ********************************************************/

/**
 * Destructive.
 * @param l A list.
 * @param v A value to insert at the last position.
 * @post The last element of the list now points to a new node containing v.
 * @return The newly created end node, or NULL on failure.
 */
list *list_append(list *l, void *v) {
  list *end = NULL;

  while (list_cdr(l)) {
    l = list_cdr(l);
  }

  if (list_car(l) == NULL) {
    list_set_car(l, v);
    return l;
  }
  else {
    if ( ! (end = list_cons(v, null_list())))
      goto emem;
    list_set_cdr(l, end);
    return end;
  }

 emem:
  free(end);
  return NULL;
}



/********************************************************
 *
 *	DESTRUCTIVE UTILITY LIST FUNCTIONS
 *
 ********************************************************/

/**
 * Alias of:
 * l = list_cons(v, l);
 * @param l A list to become the cdr.
 * @param v A value to become the car.
 * @return A list composed of v and l or NULL on failure
 */
list *list_push(list *l, void *v) {
  list *n;

  if (list_is_null(l)) {
    printf("Warning: pushing to null list.\n");
    return NULL;
  }
  if (list_is_empty(l)) {
    list_set_car(l, v);
    return l;
  }
  
  if (! (n = list_create()))
    goto emem;
  list_set_car(n, list_car(l));
  list_set_cdr(n, list_cdr(l));
  
  list_set_car(l, v);
  list_set_cdr(l, n);
  return l;

 emem:
  free(n);
  return NULL;
}

/**
 * Destructive.
 * Convenience function.
 * Opposite of list_push().
 * @param l A list.
 * @return The car of the old list.
 * @post The list is now the cdr of the original list.
 */
void *list_pop(list *l) {
  void *v;
  list *n;

  if (list_is_null(l)) {
    return NULL;
  }
  if (list_is_empty(l)) {
    return NULL;
  }
  if (list_cdr(l) == NULL) {
    v = list_car(l);
    list_set_car(l, NULL);
    return v;
  }
  
  v = list_car(l);
  n = list_cdr(l);
  
  list_set_car(l, list_car(n));
  list_set_cdr(l, list_cdr(n));
  list_atom_destroy(n);
  
  return v;
}

/**
 * Private
 * @return nth atom in list.
 */
list *list_nth_atom(list *l, int i) {
  if (i == 0) {
    return l;
  }
  // tail recursion
  return list_nth_atom(list_cdr(l), --i);
}

/**
 * Removes the element at the given index
 * @return the value of the element removed, or NULL on failure.
 */
void *list_remove(list *l, int index) {
  int size;
  size = list_size(l);

  if (index < 0 ||
      index > size)
    return NULL;

  if (index == 0)
    return list_pop(l);

  if (index == size - 1) {
    list *penultimate;
    list *last;
    void *value;
	
    penultimate = list_nth_atom(l, size - 2);
    last        = list_cdr(penultimate);

    list_set_cdr(penultimate, NULL);
    value = list_car(last);
    list_atom_destroy(last);
    return value;
  }

  list *prev;
  list *next;
  list *target;
  void *value;

  prev   = list_nth_atom(l, index - 1);
  target = list_cdr(prev);
  next   = list_cdr(target);

  list_set_cdr(prev, next);
  value = list_car(target);
  list_atom_destroy(target);

  return value;
}
