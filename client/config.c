/**
 * 
 * @file config.c
 * @author Willi Ballenthin
 * @date Spring, 2010
 */


#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// Whether to print out debug messages to STDOUT
#define DEBUG 1

/// Whether to print out warning messages to STDOUT
#define WARN 1

/**
 * Prints the given warning message to STDOUT if defined.
 * @param warning The message to print.
 */ 
void warn(const char *warning) {
  if (WARN) {
    printf("Warning: %s\n", warning);

  }
  return;
}

/**
 * Prints the given debug message to STDOUT if defined.
 * @param debug The message to print.
 */ 
void debug(const char *debug) {
  if (DEBUG) {
    printf("Debug: %s\n", debug);
  }
  return;
}

/**
 * Given a file, and key, return the first matching config value.
 * @param filename The config file.
 * @param key The key of which the value to find.
 * @return A newly allocated string, or NULL on failure.
 */
char *config_get(const char* filename, const char* key) {
  list * l;
  char *value;
  
  l = config_get_all(filename, key);
  if ( ! l) {
    warn("unable to read config file.");
    return NULL;
  }

  value = list_pop(l);

  while ( ! list_is_empty(l)) {
    free(list_pop(l));
  }

  list_destroy(l);

  return value;
}


/**
 * Given a filename and key, return all matching config values.
 * @param filename The filename of the configuration file.
 * @param key The key to which to match configuration definitions.
 * @return A newly allocated list of newly allocated values, or NULL on failure.
 */
list *config_get_all(const char* filename, const char* key) {
  char buffer[1024];
  FILE *fp;
  list *l;

  fp = fopen(filename, "r");
  if ( ! fp ) {
    warn("unable to open config file.");
    return NULL;
  }

  l = list_create();
  if ( ! l ) {
    warn("memory exhausted.");
    fclose(fp);
    return NULL;
  }

  while(!feof(fp)) {
    if(fgets(buffer, 1023, fp)) {
      if (buffer[0] == '#')
	continue;
	  
      if (strstr(buffer, key)) {
	char *p;
	char *value;
	char fragment1[1024] = { '\0' };
	char fragment2[1024] = { '\0' };
	int len;

	p = buffer;
	while (*p != '\0' && *p != '=')
	  p++;

	if (*p == '\0')
	  continue;
	strncpy(fragment1, buffer, p - buffer);
	fragment1[p - buffer] = '\0';

	if (strncmp(fragment1, key, strlen(key)) != 0)
	  continue;
		
	if (*(p + 1) == '\0')
	  continue;

	len = strlen(p) - 2;
	if (len < 0)
	  continue;
		
	strncpy(fragment2, p + 1, strlen(p) - 2);
	fragment2[strlen(p - 2)] = '\0';
		
	value = strdup(fragment2);
	if ( ! value ) {
	  warn("memory exhausted.");
	  fclose(fp);
	  return NULL;
	}

	list_append(l, value);
      }
    }
  }

  fclose(fp);
  return l;
}

