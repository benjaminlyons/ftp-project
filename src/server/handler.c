#include "server.h"
#include "macros.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int handle_put(Request *r, char* filename){
	log("put %s", filename);
	
	/* append the rootpath to the filename */
	char filepath[4096] = {0};
	char slash[2] = "/\0";
	strcat(filepath, RootPath);
	strcat(filepath, slash);
	strcat(filepath, filename);

	/* create a file stream to write */
	FILE* fs = fopen(filepath, "w");
	if(!fs)		return -1;	

	/* prepare to receive the file */
	char buffer[BUFSIZ];

	/* get information about the size of the file */
	char* size_string;
	long int size;
	if(!fgets(buffer, BUFSIZ, r->file))	goto failure;
	strtok(buffer, " ");
	size_string = strtok(NULL, " ");
	if(!size_string)	goto failure;
	size = atol(size_string);
	log("File size: %ld", size);

	/* now we read the file */
	size_t nread_total = 0;
	size_t nwritten_total = 0;
	size_t read = 0;
	size_t written = 0;

	while(nread_total < size){
		if((read = fread(buffer, sizeof(char), min(BUFSIZ, size-nread_total), r->file)) < 0){
			debug("Reading error: %s", strerror(errno));
			goto failure;
		}
		nread_total += read;

		while(nwritten_total < nread_total){
			if((written = fwrite(buffer, sizeof(char), nread_total - nwritten_total, fs)) < 0){
				debug("writing error: %s", strerror(errno));
				goto failure;
			}
			nwritten_total += written;
		}
	}
	fflush(fs);
	fclose(fs);
	return 0;

failure:
	return -1;

}

int handle_get(Request *r, char* filepath){
	log("inside get handler");
	char* request_path = determine_request_path(filepath);	
	if(!request_path){
		debug("Invalid request path!");
		return -1;
	}

	struct stat st;
	if(stat(request_path, &st) < 0){
		debug("Error with stat: %s", strerror(errno));
		goto failure;
	}

	if(!(st.st_mode & S_IFMT & S_IFREG)){
		debug("Not a regular file");
		goto failure;
	}

	FILE *fs;
	char buffer[BUFSIZ];
	size_t nread;

	/* open file for reading */
	log("Open file for reading");
	fs = fopen(request_path, "r");
	if(!fs){
		debug("Failure opening file");
		goto failure;
	}

	//write the size of the file to the socket
	fprintf(r->file, "File-size: %ld\n", st.st_size);
	log("File-size: %ld", st.st_size);

	/* read from file and write to the socket */
	log("Writing to the socket");
	while((nread = fread(buffer, sizeof(char), BUFSIZ, fs))){
		fwrite(buffer, sizeof(char), nread, r->file);
	}

	if(ferror(fs)){
		fclose(fs);
		goto failure;
	}
	fclose(fs);
	fflush(r->file);
	free(request_path);
	return 0;
failure:
	free(request_path);
	return -1;
}

// this takes the command received from the client
// and performs the actual operation
// returns -1 on failure, -2 to exit
int process_command(Request *r, char* command){
	int result = -1;
	log("processing the command: %s", command);
	if(!command)	return -1;	

	// get the type of command
	char *type = strtok(command, " ");
        if(!type)	return -1;

	char* param = strtok(NULL, " ");	
	if(streq(type, "quit") || streq(type, "QUIT")){
		return -2;
	}
	else if(streq(type, "get")){
		if(!param)	return -1;
		log("params: %s", param);
		result = handle_get(r, param);
	} else if(streq(type, "put")){
		if(!param)	return -1;
		log("params: %s", param);
		result = handle_put(r, param);
	}
	return result;
}
//
// this handles the request for the client
// Basically, we need to loop and continuously
// wait for a command from the client
// When a command is entered, we process the command
// and return the result
int handle_request(Request *r){
	log("Handling request");
	int result = 0;
	while(true){
		char buffer[BUFSIZ] = {0};
		// wait for input to be received
		log("waiting for input");
		if(!fgets(buffer, BUFSIZ, r->file)){
			goto failure;	
		}
		//remove newline. note that newline is
		//necessary for fgets to work
		chomp(buffer);

		log("got my input");
		result = process_command(r, buffer);
		if(result == -2)	break;
	}
	return 0;
failure:
	log("failed");
	return -1;
}
