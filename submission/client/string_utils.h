#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include "list.h"

#include <stdlib.h>
#include <string.h>


/// Given a string an set of delimiting characters, return a list
/// of string.
list *string_split(char *str, char *delim);


/// Given a list of strings, return a string glued together by GLUE.
char *string_join(list *strings, char *glue);

#endif // define
