#include "client.h"
#include "macros.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

void usage(char* progname, int status){
	fprintf(stderr, "Usage  %s host port\n", progname);
	exit(status);
}

FILE *socket_dial(const char *host, const char *port){
	struct addrinfo* results;
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM
	};
	int status;
	if((status = getaddrinfo(host, port, &hints, &results)) != 0){
		fprintf(stderr, "ERROR: %s\n", gai_strerror(errno));
		return NULL;	
	}

	int cfd = -1;
	for(struct addrinfo* p = results; p != NULL && cfd < 0; p = p->ai_next){
		if((cfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
			fprintf(stderr, "Unable to make socket: %s\n", strerror(errno));
			continue;
		}
		if(connect(cfd, p->ai_addr, p->ai_addrlen) < 0){
			fprintf(stderr, "Unable to connect to %s:%s: %s\n", host, port, strerror(errno));
			close(cfd);
			cfd = -1;
			continue;
		}
	}

	freeaddrinfo(results);

	if(cfd < 0){
		return NULL;
       	}

	FILE *client_file = fdopen(cfd, "w+");
	if(client_file == NULL){
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		close(cfd);
		return NULL;
	}
	return client_file;
}
int main(int argc, char* argv[]){
	char* HOST = "localhost";
	char* PORT = "9000";

	if(argc == 3){
		HOST = argv[1];
		PORT = argv[2];
	} else if (argc != 1){
		usage(argv[0], EXIT_FAILURE);
	}

	FILE* client_file = socket_dial(HOST, PORT);
	if(!client_file)	return EXIT_FAILURE;

	/*
	char* filepath = "testfile";
	log("putting");
	handle_put_command(client_file, filepath);
	handle_ls_command(client_file);
	remove(filepath);
	log("getting");
	handle_get_command(client_file, filepath);
	log("deleting");
	handle_delete_command(client_file, filepath);
	handle_ls_command(client_file);
	*/

	handle_pwd_command(client_file);
	handle_cd_command(client_file, "test");
	handle_pwd_command(client_file);
	handle_cd_command(client_file, "..");
	handle_pwd_command(client_file);
	handle_mkdir_command(client_file, "test2dir");
	handle_cd_command(client_file, "test2dir");
	handle_pwd_command(client_file);

	return 0;
}
