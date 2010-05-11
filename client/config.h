
#ifndef _config_h_
#define _config_h_

#include "list.h"

/**
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


/**
 *
 *
 * ACTION --> ( "/dev/kiosk?idTag=" | "/dev/gumstix?idTag=" )
 * SERVER --> URL:PORT
 * MODE   --> ( "client" | "kiosk" )
 *
 * There should only be one ACTION.
 * There should only be one MODE.
 * There may be multiple SERVERs specified.
 *
 */

/**
 * In the case of multiple matching definitions, the first
 * encountered definition is returned.
 */
char *config_get(const char *filename, const char *key);

list *config_get_all(const char *filename, const char *key);


#endif