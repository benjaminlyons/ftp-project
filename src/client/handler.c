#include "macros.h"
#include "client.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

// deletes file on the server
int handle_delete_command(FILE* socket_stream, char* filepath){
	fprintf(socket_stream, "delete %s\n", filepath);
	return 0;
}

// uploads the file to the server
int handle_put_command(FILE* socket_stream, char* filepath){
	/* get filename */
	char* filename = basename(filepath);
	if(!filename)	return -1;

	/* send request to server */
	fprintf(socket_stream, "put %s\n", filename);

	/* stat the file to get the size */
	struct stat s;
	if(stat(filepath, &s)){
		fprintf(stderr, "Stat error: %s\n", strerror(errno));
		return -1;
	}

	/* check to make sure it is a valid file */
	if(!(s.st_mode & S_IFMT & S_IFREG)){
		fprintf(stderr, "Not a regular file");
		return -1;
	}

	/* send file size */
	fprintf(socket_stream, "File-size: %ld\n", s.st_size);
	fflush(socket_stream);

	/* open file stream */
	FILE* f = fopen(filename, "r");
	if(!f)	return -1;

	/* now, we must write the file */
	char buffer[BUFSIZ];
	size_t nread;
	size_t nwritten_total;
	size_t nwritten;
	while( (nread = fread(buffer, sizeof(char), BUFSIZ, f))){
		if(nread < 0){
			fprintf(stderr, "Error with reading file: %s\n", strerror(errno));
			return -1;
		}
		nwritten_total = 0;
		while(nwritten_total < nread){
			if((nwritten = fwrite(buffer, sizeof(char), nread, socket_stream)) < 0){
				fprintf(stderr, "Error with writing file: %s\n", strerror(errno));	
				return -1;
			}
			nwritten_total += nwritten;
		}
	}

	fflush(socket_stream);
	fclose(f);
	return 0;
}
// downloads the file asked for by the get command
// returns -1 on failure
int handle_get_command(FILE* socket_stream, char* filepath){
	log("get %s", filepath);
	fprintf(socket_stream, "get %s\n", filepath);
	fflush(socket_stream);

	char buffer[BUFSIZ];
	char* size_string;
	long int size;
	
	// open the file
	// create it if it does not exist
	char* filename = basename(filepath);
	if(!filename)	return -1;
	FILE* fs = fopen(filename, "w+");
	if(!fs)	return -1;

	// first get the information about the size of the file
	if(!fgets(buffer, BUFSIZ, socket_stream))	goto failure;
	strtok(buffer, " ");
	size_string = strtok(NULL, " ");
	if(!size_string)	goto failure;
	size = atol(size_string);
	log("File size: %ld", size);
	if(size < 0){
		fprintf(stderr, "ERROR WITH GET\n");
		goto failure;
	}

	// now we must read the file
	size_t nread_total = 0;
	size_t nwritten_total = 0;
	size_t read = 0;
	size_t written = 0;
	while(nread_total < size){
		if( (read = fread(buffer, sizeof(char), min(BUFSIZ, size - nread_total), socket_stream)) < 0){
			debug("reading error: %s", strerror(errno));
			goto failure;
		}
		nread_total += read;
		
		while(nwritten_total < nread_total){
			if( (written = fwrite(buffer, sizeof(char), nread_total - nwritten_total, fs)) < 0){
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
	fclose(fs);
	return -1;
}

