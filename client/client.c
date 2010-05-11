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
#include "config.h"
#include "string_utils.h"

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
	

	
	list *servers;
	struct client_config client_config;
	char *ret;
	list *l, *temp;

	servers = list_create();
	if ( ! servers ) {
		printf("Error: Memory exhausted.\n");
		return -ENOMEM;
	}
	
	client_config.config_file = ".rfid_client_config";
	
	ret = config_get(client_config.config_file, "mode");
	if ( ret ) {
	  client_config.mode = ret;
	}
	else {
	  client_config.mode = strdup("client");
	}

	ret = config_get(client_config.config_file, "action");
	if ( ret ) {
	  client_config.action = ret;
	}
	else {
	  client_config.action = strdup("client");
	}

	l = config_get_all(client_config.config_file, "server");
	list_foreach_entry(l, temp, char *, ret) {
	  struct rfid_server_info *server_info;
	  list *fragments;
	  list *a;
	  char *s;
	  
	  server_info = malloc(1 * sizeof(struct rfid_server_info));
	  if ( ! server_info ) {
		printf("Error: Memory exhausted.\n");
		return -ENOMEM;
	  }
	  pthread_mutex_init(&(server_info->lock), NULL);
	  
	  fragments = string_split(ret, ":");
	  if (list_size(fragments) == 2) {
		int port;
		char *aport;

		server_info->url      = list_nth(fragments, 0);
		aport = list_nth(fragments, 1);
		port  = atoi(aport);

		if (port == 0) {
		  printf("Warning: invalid port in config file. Assuming port 80.");
		  port = 80;
		}
		server_info->port     = port;
		server_info->stable   = 1;
		server_info->last_tag = NULL;

		list_push(servers, server_info);
	  }
	  else if (list_size(fragments) == 1) {
		server_info->url      = list_nth(fragments, 0);
		server_info->port     = 80;
		server_info->stable   = 1;
		server_info->last_tag = NULL;

		list_push(servers, server_info);
	  }
	  else {
		printf("Warning: junk in config file. Ignoring.\n");
		free(server_info);
	  }

	  list_foreach_entry(fragments, a, char *, s) {
		free(s);
	  }
	  list_destroy_deep(fragments);
	  free(server_info);
	  
	  free(ret);
	}
	list_destroy_deep(l);
	
	
	
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



