#include "client.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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
static void client_callback(AvahiClient *c, AvahiClientState state, void *
userdata) {
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
