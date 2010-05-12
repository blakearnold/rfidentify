

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 1
#define WARN 1

void warn(const char *warning) {
  if (WARN) {
    printf("Warning: %s\n", warning);

  }
  return;
}

void debug(const char *debug) {
  if (DEBUG) {
	printf("Debug: %s\n", debug);
  }
  return;
}

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








