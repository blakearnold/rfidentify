#include "client.h"
#include <stdio.h>

int main() {
  list *ret = find_all_readers();
  printf("rfid readers attached: %d\n", list_size(ret));

  struct reader *reader = list_pop(ret);
  if (reader_connect(reader, 1) == RC_SUCCESS)
	printf("connected\n");
  else
	printf("not connected\n");

  // Poll for tags (ISO15693)
  list *tagList = list_create();
  int tagCnt = reader_poll15693(reader, tagList);
  if(tagCnt < 0){
	printf("Error polling reader\n");
	return -1;
  }

  // Iterate through TagList and print tag IDs.
  if (list_size(tagList) == 0) {
	printf("No tags found\n");
  }
  while (list_size(tagList) > 0) {
	struct tag *t = list_pop(tagList);
	printf("tag: %s\n", t->id);
  }

  return 1;
}



