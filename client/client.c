#include "client.h"
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>



/**
 * Called whenever a service has been resolved successfully or timed out
 */
static void resolve_callback(
    AvahiServiceResolver *r,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiClient     *c;
    AvahiSimplePoll *simple_poll;
	struct rfid_server_info *server_info;

    assert(r);

    c           = ((struct avahi_callback_params*)userdata)->client;
    simple_poll = ((struct avahi_callback_params*)userdata)->poll;
	server_info = ((struct avahi_callback_params*)userdata)->server_info;


    switch (event) {
    case AVAHI_RESOLVER_FAILURE:
        break;

    case AVAHI_RESOLVER_FOUND: {
        char ip[AVAHI_ADDRESS_STR_MAX];

        avahi_address_snprint(ip, sizeof(ip), address);
		printf("Found RFID server at %s\n", ip);
		
		pthread_mutex_lock(&(server_info->lock));
		free(server_info->url);
		server_info->url = strdup(ip);
		if ( ! server_info->url) {
		  printf("Memory exhausted.\n");
		  // not much we can do...
		}
		server_info->port = port; 
		pthread_mutex_unlock(&(server_info->lock));
		
    } // end case
    } // end switch

    avahi_service_resolver_free(r);
}

/**
 * Called whenever a new services becomes available
 * on the LAN or is removed from the LAN
 */
static void browse_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void* userdata)
{
    AvahiClient     *c;
    AvahiSimplePoll *simple_poll;
	struct rfid_server_info *server_info;

    assert(b);

    c           = ((struct avahi_callback_params*)userdata)->client;
    simple_poll = ((struct avahi_callback_params*)userdata)->poll;
	server_info = ((struct avahi_callback_params*)userdata)->server_info;

    switch (event) {
    case AVAHI_BROWSER_FAILURE:

        printf("(Browser) %s\n",
				avahi_strerror(avahi_client_errno(
				                   avahi_service_browser_get_client(b))));
								   
        avahi_simple_poll_quit(simple_poll);
        return;

    case AVAHI_BROWSER_NEW:
        if ( ! (avahi_service_resolver_new(c,
										   interface,
										   protocol,
										   name,
										   type,
										   domain,
										   AVAHI_PROTO_UNSPEC,
										   0,
										   resolve_callback,
										   userdata))) {
            printf("Failed to resolve service '%s': %s\n",
					name,
					avahi_strerror(avahi_client_errno(c)));
		}
        break;

    case AVAHI_BROWSER_REMOVE:
		// should remove url from server_info here.
		// have to do some matching against name and type and domain
        printf("(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n",
				name,
				type,
				domain);
        break;

    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        break;
    }
}

/**
 * Called whenever the client or server state changes.
 */
static void client_callback(AvahiClient *c, AvahiClientState state, void * userdata) {
    assert(c);

    if (state == AVAHI_CLIENT_FAILURE) {
        printf("Server connection failure: %s\n",
				avahi_strerror(avahi_client_errno(c)));
        avahi_simple_poll_quit((AvahiSimplePoll *)userdata);
    }
}

void *avahi_function(void *args) {
    AvahiSimplePoll *simple_poll = NULL;
    AvahiClient *client          = NULL;
    AvahiServiceBrowser *sb      = NULL;
	struct rfid_server_info *server_info = args;
    int error;
    struct avahi_callback_params abcp;

    if (!(simple_poll = avahi_simple_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        pthread_exit(NULL);
    }

    client = avahi_client_new(avahi_simple_poll_get(simple_poll),
                              0,
                              client_callback,
                              simple_poll,
                              &error);

    if (!client) {
        fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
		if (simple_poll)
			avahi_simple_poll_free(simple_poll);
		pthread_exit(NULL);
    }

    abcp.poll   = simple_poll;
    abcp.client = client;
	abcp.server_info = server_info;

    if ( ! (sb = avahi_service_browser_new(client,
                                           AVAHI_IF_UNSPEC,
                                           AVAHI_PROTO_UNSPEC,
                                           "_rfid-srv._tcp",
                                           NULL,
                                           0,
                                           browse_callback,
                                           &abcp))) {
        printf("Failed to create service browser: %s\n",
                avahi_strerror(avahi_client_errno(client)));
		if (client)
			avahi_client_free(client);
		if (simple_poll)
			avahi_simple_poll_free(simple_poll);
		pthread_exit(NULL);
    }

    // main loop
    avahi_simple_poll_loop(simple_poll);

    pthread_exit(NULL);
}

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

int main() {
    pthread_t reader_thread;
    pthread_t avahi_thread;
	
	struct rfid_server_info server_info;
	pthread_mutex_init(&(server_info.lock), NULL);
	server_info.url      = NULL;
	server_info.last_tag = NULL;
    if (pthread_create(&reader_thread, NULL, &reader_function, &server_info)) {
        printf("Error: Creation of reader thread failed.\n");
        return -1;
    }
    if (pthread_create(&avahi_thread, NULL, &avahi_function, &server_info)) {
        printf("Error: Creation of mDNS thread failed.\n");
        return -1;
    }

    pthread_join(reader_thread, NULL);
    pthread_join(avahi_thread,  NULL);
	
	return 0;
}



