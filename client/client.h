/**
 * client.h
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


/**
 * Defines a relevant configuration of an RFID server.
 */
struct rfid_server_info {
	/**
	 * As the url may be updated at any time, we must lock it
	 * before modification or reading.
	 */
	pthread_mutex_t lock;
	char           *url;       ///< RFID server url.
	uint16_t 	    port;      ///< RFID server port.
	char           *last_tag;  ///< ID of last tag read.
};

/**
 * Defines a client configuration file.
 */
struct client_config {
	char *config_file;  ///< Location of configuration file.
	char *mode;         ///< Current mode of client.
	char *url_dev;      ///< Server REST action.
};

/**
 * Aggregation of various parameters to Avahi callbacks.
 */
struct avahi_callback_params {
    AvahiSimplePoll *poll;
    AvahiClient     *client;
	struct rfid_server_info *server_info;
};

void *reader_function(void *args);
void *avahi_function(void *args);






