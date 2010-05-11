
#ifndef _config_h_
#define _config_h_

#include "list.h"

/*
 * A config file looks something like this:
 * 
 * a LINE is less than 1023 characters
 *
 * STRING        --> any stream of characters, including nil
 * COMMENT       --> # STRING
 * KEY           --> WORD
 * VALUE         --> STRING
 * DEFINITION    --> KEY=VALUE
 * LINE          --> ( COMMENT | DEFINITON | nil )\n
 * CONFIGURATION --> LINE*
 *
 */

char *config_get(const char *filename, const char *key);

list *config_get_all(const char *filename, const char *key);


#endif