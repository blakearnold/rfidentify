#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include "list.h"

#include <stdlib.h>
#include <string.h>

list *string_split(char *str, char *delim);

char *string_join(list *strings, char *glue);


#endif // define