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


void *reader_function(void *args) {
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
  pthread_exit(NULL);
}




int main() {
  pthread_t reader_thread;

  if (pthread_create(&reader_thread, NULL, &reader_function, NULL)) {
	printf("Error: Creation of reader thread failed.\n");
	return -1;
  }
  pthread_join(reader_thread, NULL);

  return 1;
}



