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

/* this function runs the cli for the client */
void client_cli(FILE* socket_stream){
	char *PROMPT = "ftp> ";
	char buffer[BUFSIZ];
	while(true){
		fprintf(stdout, "%s", PROMPT);	

		/* read from stdin */
		if(!fgets(buffer, BUFSIZ, stdin)){
			fprintf(stdout, "\n");
			break;
		}

		/* remove the new line */
		chomp(buffer);
		
		fprintf(stdout, "\n");
		handle_command(socket_stream, buffer);
	}
}
