/**
 * rfid_reader_handler.c
 *
 * The RFID reader is discovered, initialized, and
 * polled every few seconds.
 * Upon reading a tag, a query is made to a server.
 *
 * @author Willi Ballenthin
 * @date   Spring, 2010
 *
 */

#include "client.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

/// Amount of time in seconds between polls to RFID reader.
#define SLEEP_BTWN_POLL   1

/// Amount of time in seconds between polls to USB for RFID reader.
#define SLEEP_BTWN_SEARCH 1

/**
 * This function is invoked as a callback upon the reading
 * of a tag.
 * @param tag A string containing the tag id.
 * @param servers A list of server configurations.
 * @param 0 on ok, negative integer otherwise.
 */
int reader_handle_tag(const char *tag,
		      struct client_config *config) {
  CURL    *curl;
  CURLcode res;
  char    *action;
  char    *target;
  int      size;
  list    *temp;
  struct rfid_server_info *server_info;

  action = config->action;

  // if the tag has already been read, dont do anything
  if ( config->last_tag && strcmp(config->last_tag, tag) == 0) {
    return 0;
  }
  else {
    free(config->last_tag);
    config->last_tag = strdup(tag);
    if ( ! config->last_tag) {
      printf("Error: memory exhausted.\n");
      return -ENOMEM;
    }
  }

  printf("tag: %s\n", tag);

  list_foreach_entry(config->servers, temp, struct rfid_server_info *, server_info) {

    pthread_mutex_lock(&(server_info->lock));

    // if there is no destination
    // do nothing and return failure (loop will reread)
    if (server_info->url == NULL) {
      pthread_mutex_unlock(&(server_info->lock));
      continue;
    }

    // compose target url
    size   = strlen(server_info->url) + strlen(tag) + strlen(action) + 1;
    target = malloc(sizeof(char) * size );
    if ( ! target) {
      printf("Memory exhausted.\n");
      pthread_mutex_unlock(&(server_info->lock));
      continue;
    }
    target[0] = '\0';
    strcpy(target, server_info->url);
    printf("to server url: %s\n", server_info->url);
    strcat(target, action);
    strcat(target, tag);

    printf("query to %s\n", target);
    fflush(stdout);

    // make request
    curl = curl_easy_init();
    if ( ! curl) {
      free(target);
      pthread_mutex_unlock(&(server_info->lock));
      continue;
    }
    curl_easy_setopt(curl, CURLOPT_URL, target);
    curl_easy_setopt(curl, CURLOPT_PORT, (long)server_info->port);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    free(target);

    pthread_mutex_unlock(&(server_info->lock));
  }
  return 0;
}

/**
 * Repeatedly queries the device for RFID tags.
 * @see SLEEP_BTWN_POLL
 * @param reader The RFID reading device.
 * @param servers A list of server configurations.
 * @return Well it shouldnt. If it does, there's an error.
 */
int reader_poll_loop(struct reader *reader,
		     struct client_config *config) {
  list *tags;
  tags = list_create();
  if ( ! tags) {
    printf("Memory exhausted.\n");
    return ENOMEM;
  }

  while (1) {
    int num_tags = reader_poll15693(reader, tags);
    if (num_tags < 0) {
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

      if (reader_handle_tag(t->id, config)) {
	printf("Warning: tag handler failed.\n");
      }

      free(t->id);
      free(t);
    }
    sleep(SLEEP_BTWN_POLL);
  }
}

/**
 * PThread.
 * Initializes an RFID device, if it exists, and begins a
 * tag reading loop.
 * @see SLEEP_BTWN_SEARCH
 * @param args a list of struct rfid_server_info.
 * @return Should  not return, if so, error.
 * TODO abstract RFID differences from this implementation
 */
void *reader_function(void *args) {
  list *readers;
  struct reader *reader;
  struct client_config *config;

  config = args;
	
  readers = list_create();
  if ( ! readers) {
    printf("Memory exhausted.\n");
    pthread_exit(NULL);
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

  reader_poll_loop(reader, config);
  // only get here on error

  free(reader->ftdic);
  free(reader);
  pthread_exit(NULL);
}
