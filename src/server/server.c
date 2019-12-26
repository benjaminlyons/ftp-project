#include "server.h"
#include "macros.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

// default port number
char *Port = "9000";
char *RootPath = "sandbox";

void usage (const char *progname, int status){
	fprintf(stderr, "Usage: %s portnumber\n", progname);
	exit(status);
}

void server(int server_fd){
	/* accept and handle request from client*/
	while(true){
		Request *r = accept_request(server_fd);
		handle_request(r);
		free_request(r);
	}	

	close(server_fd);
}

int main(int argc, char* argv[]){
	// if invalid command arguments
	if(argc > 2){
		usage(argv[0], EXIT_FAILURE);	
	}
	if(argc == 2){
		Port = argv[1];
	}

	// allocate a socket and listen to it
	int server_fd = socket_listen(Port);
	if(server_fd < 0){
		debug("Error with socket listen");
		return EXIT_FAILURE;
	}

	RootPath = realpath(RootPath, NULL);
	log("Listening on port %s", Port);
	debug("RootPath = %s", RootPath);

	server(server_fd);
	return EXIT_SUCCESS;
}
