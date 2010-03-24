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
        fprintf(stderr,
                "(Resolver) Failed to resolve service '%s'"
                " of type '%s' in domain '%s': %s\n",
                name,
                type,
                domain,
                avahi_strerror(avahi_client_errno(
                                   avahi_service_resolver_get_client(r))));
        break;

    case AVAHI_RESOLVER_FOUND: {
        char ip[AVAHI_ADDRESS_STR_MAX], *t;

        fprintf(stderr,
                "Service '%s' of type '%s' in domain '%s':\n",
                name,
                type,
                domain);

        avahi_address_snprint(ip, sizeof(ip), address);
        t = avahi_string_list_to_string(txt);
        fprintf(stderr,
                "\t%s:%u (%s)\n"
                "\tTXT=%s\n"
                "\tcookie is %u\n"
                "\tis_local: %i\n"
                "\tour_own: %i\n"
                "\twide_area: %i\n"
                "\tmulticast: %i\n"
                "\tcached: %i\n",
                host_name,
				port,
				ip,
                t,
                avahi_string_list_get_service_cookie(txt),
                !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
                !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
                !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
                !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
                !!(flags & AVAHI_LOOKUP_RESULT_CACHED));

		pthread_mutex_lock(&(server_info->lock));
		free(server_info->url);
		server_info->url = strdup(ip);
		if ( ! server_info->url) {
		  printf("Memory exhausted.\n");
		  // not much we can do...
		}
		pthread_mutex_unlock(&(server_info->lock));
				

        avahi_free(t);
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

        fprintf(stderr,
				"(Browser) %s\n",
				avahi_strerror(avahi_client_errno(
				                   avahi_service_browser_get_client(b))));
								   
        avahi_simple_poll_quit(simple_poll);
        return;

    case AVAHI_BROWSER_NEW:
        fprintf(stderr,
				"(Browser) NEW: service '%s' of type '%s' in domain '%s'\n",
				name,
				type,
				domain);

        /* We ignore the returned resolver object. In the callback
           function we free it. If the server is terminated before
           the callback function is called the server will free
           the resolver for us. */

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
            fprintf(stderr,
					"Failed to resolve service '%s': %s\n",
					name,
					avahi_strerror(avahi_client_errno(c)));
		}
        break;

    case AVAHI_BROWSER_REMOVE:
		// should remove url from server_info here.
		// have to do some matching against name and type and domain
        fprintf(stderr,
				"(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n",
				name,
				type,
				domain);
        break;

    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        fprintf(stderr,
				"(Browser) %s\n",
				event == AVAHI_BROWSER_CACHE_EXHAUSTED
				              ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
        break;
    }
}

/**
 * Called whenever the client or server state changes.
 */
static void client_callback(AvahiClient *c, AvahiClientState state, void * userdata) {
    assert(c);

    if (state == AVAHI_CLIENT_FAILURE) {
        fprintf(stderr,
				"Server connection failure: %s\n",
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
    int ret = 1;
    struct avahi_callback_params abcp;

    if (!(simple_poll = avahi_simple_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        goto fail;
    }

    client = avahi_client_new(avahi_simple_poll_get(simple_poll),
                              0,
                              client_callback,
                              simple_poll,
                              &error);

    if (!client) {
        fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
        goto fail;
    }

    abcp.poll   = simple_poll;
    abcp.client = client;
	abcp.server_info = server_info;

    if ( ! (sb = avahi_service_browser_new(client,
                                           AVAHI_IF_UNSPEC,
                                           AVAHI_PROTO_UNSPEC,
                                           "_rfid._server._tcp",
                                           NULL,
                                           0,
                                           browse_callback,
                                           &abcp))) {
        fprintf(stderr,
                "Failed to create service browser: %s\n",
                avahi_strerror(avahi_client_errno(client)));
        goto fail;
    }

    // main loop
    avahi_simple_poll_loop(simple_poll);

    ret = 0;

    pthread_exit(NULL);
    //return ret;

fail:

    /* Cleanup things */
    if (sb)
        avahi_service_browser_free(sb);

    if (client)
        avahi_client_free(client);

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    pthread_exit(NULL);
    //return 1;
}

int reader_handle_tag(const char *tag,
					  struct rfid_server_info *server_info) {
    CURL *curl;
    CURLcode res;
    char *action = "/read?";
    char *target;
	int size;

    printf("tag: %s\n", tag);

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
	server_info.url = NULL;

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



