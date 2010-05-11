
#ifndef _config_h_
#define _config_h_

#include "list.h"

char *config_get(const char *filename, const char *key);

list *config_get_all(const char *filename, const char *key);


#endif