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

/* use the logging stuff from dr. bui's code */
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define fatal(M, ...) fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE)
#define log(M, ...)	fprintf(stderr, "[%5d] LOG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

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

#define streq(a, b) (strcmp((a), (b)) == 0)
#define chomp(s) (s)[strlen(s) - 1] = '\0'
