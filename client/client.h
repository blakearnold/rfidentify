/**
 * @file client.h
 *
 * This application reads RFID tags and queries a
 * server with the tag ID.
 * The server is discovered using Avahi/mDNS. 
 *
 * @author Willi Ballenthin
 * @date   Spring, 2010
 */

#include "reader.h"
#include <avahi-client/client.h>
#include <avahi-common/simple-watch.h>
#include <pthread.h>



/// Defines a relevant configuration of an RFID server.
struct rfid_server_info {
  /**
   * As the url may be updated at any time, we must lock it
   * before modification or reading.
   */
  pthread_mutex_t lock;      ///< Protects modifications across threads.
  char           *url;       ///< RFID server url.
  uint16_t 	  port;      ///< RFID server port.
  int             stable;    ///< True if the URL will not change. Eg. false if it may be modified by mDNS.
  char           *name;      ///< Unique name of the given server. Provided by mDNS.
};

/// Defines a client configuration context.
struct client_config {
  char *config_file;  ///< Location of configuration file.
  char *mode;         ///< Current mode of client.
  char *action;       ///< Server REST action.
  char *last_tag;     ///< Last tag encountered by reader.
  list *servers;      ///< List of rfid_server_info structures.
  pthread_mutex_t lock; ///< Protects modifications, especially last_tag.
};


/// Aggregation of various parameters to Avahi callbacks.
struct avahi_callback_params {
  AvahiSimplePoll      *poll;   ///< Current poll object.
  AvahiClient          *client; ///< Currently working client.
  struct client_config *config; ///< Client configuration status.
};

/// RFID reading loop which polls for identifications. Look for rfid_reader_handler.c
void *reader_function(void *args);

/// mDNS handling loop which looks for RFIDentify servers. Look for avahi_dns_handler.c
void *avahi_function(void *args);

/// Configures a client given a configuration file.
int   read_config(struct client_config *client_config, const char *filename);

/// Configures static servers from a configuration file.
list *read_config_servers(struct client_config *client_config);






