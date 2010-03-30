#include "client.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#define SLEEP_BTWN_POLL   1
#define SLEEP_BTWN_SEARCH 1

/**
 * This function is invoked as a callback upon the reading
 * of a tag.
 * @param tag A string containing the tag id.
 * @param server_info Relevant server configuration details.
 * @param 0 on ok, negative integer otherwise.
 *        -1 CURL initialization failed.
 *        -2 No server specified.
 */
int reader_handle_tag(const char *tag,
					  struct rfid_server_info *server_info) {
    CURL *curl;
    CURLcode res;
    char *action = "/dev/gumstix?idTag=";
    char *target;
	int size;
	
	pthread_mutex_lock(&(server_info->lock));

	// if there is no destination
	// do nothing and return failure (loop will reread)
	if (server_info->url == NULL) {
	  pthread_mutex_unlock(&(server_info->lock));
	  return -2;
	}

	// if the tag has already been read
	// dont do anything
	if ( server_info->last_tag && strcmp(server_info->last_tag, tag) == 0) {
	  pthread_mutex_unlock(&(server_info->lock));
	  return 0;
	}

	printf("tag: %s\n", tag);

	// compose target url
	size   = strlen(server_info->url) + strlen(tag) + strlen(action) + 1;
    target = malloc(sizeof(char) * size );
    if ( ! target) {
        printf("Memory exhausted.\n");
		pthread_mutex_unlock(&(server_info->lock));
        return ENOMEM;
    }
    strcpy(target, server_info->url);
    strcat(target, action);
    strcat(target, tag);

	printf("query to %s\n", target);
	fflush(stdout);

	// make request
    curl = curl_easy_init();
    if ( ! curl) {
		free(target);
		pthread_mutex_unlock(&(server_info->lock));
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, target);
	curl_easy_setopt(curl, CURLOPT_PORT, (long)server_info->port);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    free(target);

	// update last tag info
	free(server_info->last_tag);
	server_info->last_tag = strdup(tag);

	if ( ! server_info->last_tag) {
		printf("Memory exhausted.\n");
		pthread_mutex_unlock(&(server_info->lock));
		return ENOMEM;
	}

	pthread_mutex_unlock(&(server_info->lock));
    return 0;
}

int reader_poll_loop(struct reader *reader,
					 struct rfid_server_info *server_info) {

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

			if (reader_handle_tag(t->id, server_info)) {
				// maybe this should fail/return error?
				printf("Warning: tag handler failed.\n");
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
	struct rfid_server_info *server_info = args;
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

    reader_poll_loop(reader, server_info);
    // only get here on error

    free(reader->ftdic);
    free(reader);
    pthread_exit(NULL);
}
