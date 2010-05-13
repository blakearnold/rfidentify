/**
 * @file client.c
 *
 * This application reads RFID tags and queries a
 * server with the tag ID.
 * The server is discovered using Avahi/mDNS.
 *
 * @author Willi Ballenthin
 * @date   Spring, 2010
 *
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

/**
 * Configures a client given a configuration file.
 * @param client_config The destination configuration.
 * @param filename The file from which to read the config.
 * @return 0 on success. non-zero otherwise.
 */
int read_config(struct client_config *client_config, const char *filename) {
  char *ret;
  
  client_config->config_file = strdup(filename);
  client_config->last_tag    = NULL;

  pthread_mutex_init(&(client_config.lock), NULL);		

  if ( ! client_config->config_file) {
    printf("Error: memory exhausted.\n");
    return -ENOMEM;
  }
	
  ret = config_get(client_config->config_file, "mode");
  if ( ret ) {
    client_config->mode = ret;
  }
  else {
    client_config->mode = strdup("client");
  }
  
  ret = config_get(client_config->config_file, "action");
  if ( ret ) {
    client_config->action = ret;
  }
  else {
    client_config->action = strdup("client");
  }

  client_config->servers = read_config_servers(client_config);
  if ( ! client_config->servers) {
    printf("Error: memory exhausted.\n");
    return -ENOMEM;
  }

  return 0;
}

/**
 * Configures static servers from a configuration file.
 * @param client_config The config, including filename, from which to 
 *      determine the servers.
 * @return A list of servers, or NULL on failure.
 */
list *read_config_servers(struct client_config *client_config) {

  list *servers;
  list *l, *temp;
  char *ret;

  servers = list_create();
  if ( ! servers ) {
    printf("Error: Memory exhausted.\n");
    return NULL;
  }

  l = config_get_all(client_config->config_file, "server");
  list_foreach_entry(l, temp, char *, ret) {
    struct rfid_server_info *server_info;
    list *fragments;
    list *a;
    char *s;
	  
    server_info = malloc(1 * sizeof(struct rfid_server_info));
    if ( ! server_info ) {
      printf("Error: Memory exhausted.\n");
      return NULL;		  
    }
    pthread_mutex_init(&(server_info->lock), NULL);
	  
    fragments = string_split(ret, ":");
    if (list_size(fragments) == 2) {
      int port;
      char *aport;

      server_info->url      = strdup(list_nth(fragments, 0));
      printf("server: %s\n", server_info->url);
      aport = list_nth(fragments, 1);
      port  = atoi(aport);

      if (port == 0) {
	printf("Warning: invalid port in config file. Assuming port 80.");
	port = 80;
      }
      server_info->port     = port;
      server_info->stable   = 1;
      server_info->last_tag = NULL;

      server_info->name     = strdup("server_from_config");
      if ( ! server_info->name ) {
	printf("Error: Memory exhausted.\n");
	return NULL;		  
      }
		
      list_push(servers, server_info);
    }
    else if (list_size(fragments) == 1) {
      server_info->url      = strdup(list_nth(fragments, 0));
      server_info->port     = 80;
      server_info->stable   = 1;
      server_info->last_tag = NULL;
	    
      server_info->name     = strdup("server_from_config");
      if ( ! server_info->name ) {
	printf("Error: Memory exhausted.\n");
	return NULL;
      }
	    
      list_push(servers, server_info);
    }
    else {
      printf("Warning: junk in config file. Ignoring.\n");
      free(server_info);
    }

    list_foreach_entry(fragments, a, char *, s) {
      free(s);
    }
    while ( ! list_is_empty(fragments)) {
      // TODO memory error here
      //void *p = list_pop(fragments);
      //free(p);
      list_pop(fragments);
    }
    list_destroy(fragments);
	  
    free(ret);
  }
  while ( ! list_is_empty(l)) {
    // TODO memory error here
    //void *p = list_pop(fragments);
    //free(p);
    list_pop(fragments);
  }
  list_destroy(l);

  return servers;
}


int main(int argc, char **argv) {
  pthread_t reader_thread;
  pthread_t avahi_thread;
  struct client_config client_config;
  
  if (  read_config(&client_config, ".rfid_client.conf")) {
    return -1;
  }
  

  if (pthread_create(&reader_thread, NULL, &reader_function, &client_config)) {
    printf("Error: Creation of reader thread failed.\n");
    return -1;
  }

  if (pthread_create(&avahi_thread, NULL, &avahi_function, &client_config)) {
    printf("Error: Creation of mDNS thread failed.\n");
    return -1;
  }
  
  pthread_join(reader_thread, NULL);
  pthread_join(avahi_thread,  NULL);
  
  return 0;
}



