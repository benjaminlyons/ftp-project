/* this simply creates a socket, binds to it and listens to the port */

#include "server.h"
#include "macros.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/* 
 * allocates socket, binds to it, and listens to the port
 */
int socket_listen(const char *port){
	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC; // specifies type of address
	hints.ai_socktype = SOCK_STREAM; // type of socket
	hints.ai_flags = AI_PASSIVE; // just random things

	struct addrinfo *results;
	// get all of the addresses on the network that correspond
	// to hints and the port
	int status = getaddrinfo(NULL, port, &hints, &results);
	if(status != 0){
		debug("getaddrinfo failed: %s\n", gai_strerror(status));	
		return -1;
	}

	/* Now for each server entry, try to create a socket and connect to it */
	/* Note that results is a linked list */
	int socket_fd = -1;
	for(struct addrinfo *p = results; p != NULL && socket_fd < 0; p = p->ai_next){
		/* allocate the socket */
		socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socket_fd < 0){
			log("Unable to make socket: %s\n", strerror(errno));
			continue;
		}

		/* try binding the socket */
		if(bind(socket_fd, p->ai_addr, p->ai_addrlen) < 0){
			log("Unable to bind: %s\n", strerror(errno));
			close(socket_fd);
			socket_fd = -1;
			continue;
		}

		/* Listen to socket */
		if (listen(socket_fd, SOMAXCONN)){
			log("Unable to listen: %s\n", strerror(errno));
			close(socket_fd);
			socket_fd = -1;
			continue;
		}
	}
	freeaddrinfo(results);
	return socket_fd;
}

