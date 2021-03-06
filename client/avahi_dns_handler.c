/**
 * @file avahi_dns_handler.c
 *
 * The Avahi mDNS service discovery system is initialized
 * and listens for RFID servers.
 * Upon recieving an updated, the server configuration
 * information is similarly updated.
 *
 * @author Willi Ballenthin
 * @date   Spring, 2010
 *
 * @todo Multiple RFID servers are not handled correctly.
 */

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
 * Called whenever a service has been
 * resolved successfully or timed out.
 * @see simple_poll_reader.c in libavahi API
 * @param userdata An avahi_callback_params pointer containing
 *                     relevant data.
 */
void resolve_callback(
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
		      void* userdata)
{
  struct client_config *config;
  config = ((struct avahi_callback_params*)userdata)->config;

  switch (event) {
  case AVAHI_RESOLVER_FAILURE:
    break;

  case AVAHI_RESOLVER_FOUND: {
    list *temp;
    struct rfid_server_info *info;
    int new_server = 1; // this is a hack, it makes the loop easier...
	  
    char ip[AVAHI_ADDRESS_STR_MAX];

    avahi_address_snprint(ip, sizeof(ip), address);
    printf("Found RFID server at %s\n", ip);

    list_foreach_entry(config->servers, temp, struct rfid_server_info *, info) {
      if (strcmp(name, info->name) == 0 && info->stable == 0) {
	new_server = 0;
      }
    }

    if (new_server) {
      info = malloc(1 * sizeof (struct rfid_server_info));

      if ( ! info ) {
	printf("Error: memory exhausted.\n");
	avahi_service_resolver_free(r);
	return;
      }

      pthread_mutex_init(&(info->lock), NULL);		
      info->port   = port;
      info->stable = 0;

      info->url = strdup(ip);
      if ( ! info->url) {
	printf("Error: memory exhausted.\n");
	break;
      }

      info->name = strdup(name);
      if ( ! info->name) {
	printf("Error: memory exhausted.\n");
	break;
      }

      //TODO lock servers before modifying...
      list_push(config->servers, info);
    }

  } // end case
  } // end switch

  avahi_service_resolver_free(r);
  return;
}

/**
 * Called whenever a new services becomes available
 * on the LAN or is removed from the LAN.
 * @see simple_poll_reader.c in libavahi API
 * @param userdata An avahi_callback_params pointer containing
 *                     relevant data.
 */
void browse_callback(
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
  struct client_config *config;
  config = ((struct avahi_callback_params*)userdata)->config;

  switch (event) {
  case AVAHI_BROWSER_FAILURE:

    printf("(Browser) %s\n",
	   avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(b))));

    avahi_simple_poll_quit(((struct avahi_callback_params*)userdata)->poll);
    return;

  case AVAHI_BROWSER_NEW:
    if ( ! (avahi_service_resolver_new(((struct avahi_callback_params*)userdata)->client,
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
	     avahi_strerror(avahi_client_errno(((struct avahi_callback_params*)userdata)->client)));
    }
    break;

  case AVAHI_BROWSER_REMOVE: {
    printf("(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n",
	   name,
	   type,
	   domain);

    list *temp;
    struct rfid_server_info *info;
    /// @TODO this is a hack, rather than coming up with a good loop
    int new_server = 1; 
    int index      = 0;


    /// @TODO we just assume this can only match one. not ideal.
    /// @TODO we only compare against the name, with no consideration of
    ///      domain, which is bad. should come up with a hash.
    list_foreach_entry(config->servers, temp, struct rfid_server_info *, info) {
      if (strcmp(name, info->name) == 0 && info->stable == 0) {
	new_server = 0;
	break;
      }
      index++;
    }

    if ( ! new_server ) {
      list_remove(config->servers, index);
    }

				
    break;
  }

  case AVAHI_BROWSER_ALL_FOR_NOW:
  case AVAHI_BROWSER_CACHE_EXHAUSTED:
    break;
  }
  return;
}

/**
 * Called whenever the client or server state changes.
 * @param c The current client.
 * @param state The current state.
 * @param userdata Relevant data, including configuration settings.
 */
void client_callback(AvahiClient *c,
		     AvahiClientState state,
		     void *userdata)
{
  assert(c);
  
  if (state == AVAHI_CLIENT_FAILURE) {
    printf("Server connection failure: %s\n",
	   avahi_strerror(avahi_client_errno(c)));
    avahi_simple_poll_quit((AvahiSimplePoll *)userdata);
  }
  
  return;
}

/**
 * PThread.
 * Initializes an mDNS Avahi service, and listens for relevant services.
 * Updates the server info configuration as necessary.
 * @param args struct client_config, Relevant configuration settings.
 * @return Should  not return, if so, error.
 */
void *avahi_function(void *args) {
  AvahiSimplePoll *simple_poll = NULL;
  AvahiClient *client          = NULL;
  AvahiServiceBrowser *sb      = NULL;
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
    
  abcp.poll    = simple_poll;
  abcp.client  = client;
  abcp.config  = args;
    
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
