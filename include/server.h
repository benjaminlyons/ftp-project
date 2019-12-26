// this is loosely based off dr. bui's spidey project

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <unistd.h>

/* Constants */
extern char* Port;
extern char* RootPath;


typedef struct {
	int fd; // client socket fd
	FILE *file; // client file stream

	char host[NI_MAXHOST]; /* Host name of client */
	char port[NI_MAXSERV]; /* Port number of client */

} Request;

Request * accept_request(int sfd);
int handle_request(Request* request);
void free_request(Request *request);

int socket_listen(const char *port);

char* determine_request_path(const char* filename);

