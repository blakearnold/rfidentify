

#include "list.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {

  char *ret;
  list *l, *temp;


  ret = config_get("test_config", "test_key");

  if ( ! ret ) {
	printf("no results.\n");
  }
  else {
	printf("len: %d value: %s\n", strlen(ret), ret);
  }

  l = config_get_all("test_config", "test_key");

  free(ret);

  printf("---------------\n");
  
  list_foreach_entry(l, temp, char *, ret) {
	if ( ret ) {
	  printf("len: %d value: %s\n", strlen(ret), ret);
	}
  }


  return 0;
}