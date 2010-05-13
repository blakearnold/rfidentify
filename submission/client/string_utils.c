/**
 * @file string_utils.c
 * 
 * @author Willi Ballenthin
 * @date Spring, 2010
 */

#include "string_utils.h"

/**
 * Given a string an set of delimiting characters, return a list
 * of string.
 * @param strings A list of strings to join.
 * @param glue A string to be used as glue.
 * @return A new string composed of the parts provided in the list glued
 *   together by the join string, or NULL on failure.
 */
char *string_join(list *strings, char *glue) {
  list *l = strings;
  int i;
  int buflen = 0;
  int strings_length    = 0;
  int list_length = 0;
  char *text;

  while ( ! list_is_empty(l)) {
	strings_length += strlen((char *)list_car(l));
	l               = list_cdr(l);
	list_length++;
  }

  buflen  = strings_length + 1; // length + NULL
  buflen += strlen(glue) * list_length;
  if ( ! (text = (char *)malloc(sizeof(char) * buflen)))
	goto emem;
  *text = 0;

  l = strings;
  if (list_length >= 1) {
    strcat(text, (char *)list_car(l));
	l = list_cdr(l);
  }

  for (i = 1; i < list_length; i++) {
    strcat(text, glue);
    strcat(text, (char *)list_car(l));
	l = list_cdr(l);
  }

  return text;

emem:
  free(text);
  return NULL;
  
}

/**
 * split a string into a list of new substrings
 * delimited by some set of characters
 * the resulting list should be deep freed.
 * @param line The string to split.
 * @param delim A set of characters, each of which should cause a split.
 * @return New list containing the fragments, or NULL on failure.
 */
list *string_split(char *line, char *delim) {
  list *l;
  char *working_copy;
  char *fragment = NULL;

  if ( ! (l = list_create()))
	goto emem;
  if ( ! (working_copy = strdup(line)))
	goto emem;
  fragment = strtok(working_copy, delim);

  while (fragment) {
    fragment = strdup(fragment);
	if ( ! (list_push(l, fragment)))
	  goto emem;
    fragment = strtok(NULL, delim);
  }
  l = list_reverse(l);
  free(working_copy);

  return l;

emem:
  list_destroy_deep(l);
  free(working_copy);
  free(fragment);
  return NULL;
}
