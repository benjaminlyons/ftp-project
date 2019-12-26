#include "server.h"
#include "macros.h"

#include <errno.h>
#include <string.h>
/* again this is heavily based on bui's code from spidey */
/* we basically accept a request from the socket */
/* create a request struct, accept the connection, get the */
/* client's information and store it, open the socket stream for*/
/* the client. return the request struct */

Request * accept_request(int sfd){
	Request *r;
	struct sockaddr raddr;
	socklen_t rlen;

	/* allocate request struct */
	r = calloc(1, sizeof(Request));
	if(!r){
		return NULL;
	}

	/* Accept a client */
	rlen = sizeof(struct sockaddr);
	int cfd = accept(sfd, &raddr, &rlen);
	if(cfd < 0){
		debug("Unable to accept: %s", strerror(errno));
		goto failure;
	}

	/* lookup the client */
	r->fd = cfd;
	if(getnameinfo(&raddr, rlen, r->host, NI_MAXHOST, r->port, NI_MAXSERV, NI_NOFQDN) != 0){
		debug("Error with getnameinfo: %s", strerror(errno));
		goto failure;
	}

	/* Open socket stream */
	r->file = fdopen(r->fd, "r+");
	if(!r->file){
		debug("Unable to open socket stream: %s", strerror(errno));
		goto failure;
	}

	log("Accepted a request from %s:%s", r->host, r->port);
	return r;
failure:
	free(r);
	return NULL;
}

void free_request(Request *r){
	if(!r){
		return;
	}
	log("Freeing request from %s:%s", r->host, r->port);
	if(r->file){
		fclose(r->file);
	} else if(r->fd){
		close(r->fd);
	}
	free(r);
	log("Freeing successful!");
}
