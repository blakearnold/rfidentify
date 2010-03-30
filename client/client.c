/**
 * client.c
 *
 * This application reads RFID tags and queries a
 * server with the tag ID.
 * The server is discovered using Avahi/mDNS.
 *
 * @author Willi Ballenthin
 * @date   Spring, 2010
 *
 * @todo Move initial server details to configuration file.
 * @todo Combine kiosk and client mode.
 */


#include "client.h"
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

int main(int argc, char **argv) {
	//int c;
    pthread_t reader_thread;
    pthread_t avahi_thread;
	
	struct rfid_server_info server_info;
	
	/**
	struct client_config client_config;

	
	client_config.config_file = NULL;
	client_config.mode        = NULL;
	client_config.url_dev     = NULL;
	*/
	
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



