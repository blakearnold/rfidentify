#include "client.h"
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>


int handle_tag(const char *tag_id) {
  printf("tag: %s\n", tag_id);
  return 0;
}

int poll_loop(struct reader *reader) {

  list *tags;
  char *last_id = NULL;
  tags = list_create();
  if ( ! tags) {
	printf("Memory exhausted.\n");
	return ENOMEM;
  }
  
  while (1) {
	int num_tags = reader_poll15693(reader, tags);
	if(num_tags < 0){
	  printf("Error polling reader\n");
	  list_destroy(tags);
	  return EDEVERR;
	}

	if (list_size(tags) == 0) {
	  printf("No tags found\n");
	}
	
	while (list_size(tags) > 0) {
	  struct tag *t;
	  t = list_pop(tags);

	  if ( ! last_id || strcmp(last_id, t->id) != 0) {
		free(last_id);
		last_id = strdup(t->id);
		if ( ! last_id) {
		  printf("Memory exhausted.\n");
		  return ENOMEM;
		}
		
		if (handle_tag(t->id)) {
		  // maybe this should fail/return error?
		  printf("Warning: tag handler failed.\n");
		}
	  }
	  else {
		// it might make sense to sleep extra here
	  }

	  free(t->id);
	  free(t);
	}
	sleep(SLEEP_BTWN_POLL);
  }
}


void *reader_thread(void *args) {
  list *readers;
  struct reader *reader;
  readers = list_create();
  if ( ! readers) {
	printf("Memory exhausted.\n");
	pthread_exit(NULL);
	// out of memory. that sucks
  }

  while (list_size(find_all_readers(readers)) == 0 ) {
	// spin
	sleep(SLEEP_BTWN_SEARCH);
  }

  if (list_size(readers) > 1) {
	printf("Warning: Found more than one reader. Found %d readers.\n",
		   list_size(readers));
  }

  reader = list_pop(readers);
  while (reader_connect(reader, BEEP) != RC_SUCCESS) {
	printf("Unable to connect to reader.\n");
	free(reader->ftdic);
	free(reader);

	if (list_size(readers) == 0) {
	  printf("List of readers exhausted.\n");
	  // this is bad
	  pthread_exit(NULL);
	}
	reader = list_pop(readers);
  }
  printf("Connected to reader.\n");

  while (list_size(readers) > 0) {
	struct reader *r = list_pop(readers);
	free(r->ftdic);
	free(r);
  }
  list_destroy(readers);

  poll_loop(reader);

  // only get here on error
  
  free(reader->ftdic);
  free(reader);
  //pthread_exit(NULL);
  return NULL;
}




int main() {

  reader_thread(NULL);
  
  /*
  list *ret = list_create();
  find_all_readers(ret);
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
  */

  return 1;
}



