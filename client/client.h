#include "reader.h"
#include <avahi-client/client.h>
#include <avahi-common/simple-watch.h>
#include <pthread.h>


struct rfid_server_info {
  pthread_mutex_t lock;
  char           *url;
  char           *last_tag;
  uint16_t 	      port;
};

struct client_config {
	char *config_file;
	char *mode;
	char *url_dev;
};

struct avahi_callback_params {
    AvahiSimplePoll *poll;
    AvahiClient     *client;
	struct rfid_server_info *server_info;
};

void *reader_function(void *args);
void *avahi_function(void *args);






