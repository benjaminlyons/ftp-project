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

int handle_pwd(Request *r){
	fprintf(r->file, "%s\n", WorkingPath);
	fflush(r->file);
	return 0;
}
int handle_mkdir(Request *r, char* dir){
	char fullpath[4096] = {0};
	strcat(fullpath, WorkingPath);
	strcat(fullpath, SLASH);
	strcat(fullpath, dir);
	log("Mkdir %s", fullpath);
	if(mkdir(fullpath, 0700) != 0){
		fprintf(stderr, "Error, could not mkdir: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int handle_cd(Request *r, char* path){
	path = determine_request_path(path);
	if(!path){
		fprintf(stderr, "error, could not cd!\n");
		return -1;
	}
	WorkingPath = path;
	log("Cd to %s", WorkingPath);
	return 0;
}

/* sends the ls information back to the client */
int handle_ls(Request *r){
	struct dirent **entries;
	size_t n;

	struct stat st;
	char filetype = 'r';

	/* open a directory for scanning */
	n = scandir(WorkingPath, &entries, NULL, alphasort);
	if (n < 0){
		fprintf(stderr, "Error scanning diretory: %s\n", strerror(errno));
		return -1;
	}

	/* for each item in the directory, print it out */
	for(int i = 0; i < n; i++){
		if(streq(entries[i]->d_name, ".") || streq(entries[i]->d_name, "..")){
			free(entries[i]);
			continue;
		}
		
		char filepath[4096] = {0};
		strcat(filepath, WorkingPath);
		strcat(filepath, SLASH);
		strcat(filepath, entries[i]->d_name);
		if(lstat(filepath, &st) != 0){
			filetype = 'u';
		} else {
			if(S_ISREG(st.st_mode))	filetype = 'r';
			if(S_ISDIR(st.st_mode)) filetype = 'd';
			if(S_ISLNK(st.st_mode)) filetype = 'l';
		}
		
		fprintf(r->file, "%c %s\n", filetype, entries[i]->d_name);
		free(entries[i]);
	}
	free(entries);
	fprintf(r->file, "\n");
	fflush(r->file);

	return 0;

}

/* removes the file specified by filepath */
int handle_delete(Request* r, char* filepath){
	log("delete %s", filepath);

	/* make sure we have the full path to the file */
	filepath = determine_request_path(filepath);
	if(!filepath){
		fprintf(stderr, "File not found\n");
		return -1;
	}
	
	if(remove(filepath) == 0){
		log("file %s deleted", filepath);
		free(filepath);
		return 0;
	} else {
		fprintf(stderr, "Unsuccessful delete operation\n");
		free(filepath);
		return -1;
	}	
}

int handle_put(Request *r, char* filename){
	log("put %s", filename);
	
	/* append the rootpath to the filename */
	char filepath[4096] = {0}; 
       	strcat(filepath, WorkingPath);
	strcat(filepath, SLASH);
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
	if(streq(type, "quit")){
		return -2;
	} else if(streq(type, "get")){
		if(!param)	return -1;
		log("params: %s", param);
		result = handle_get(r, param);
	} else if(streq(type, "put")){
		if(!param)	return -1;
		log("params: %s", param);
		result = handle_put(r, param);
	} else if(streq(type, "delete")){
		if(!param)	return -1;
		log("params: %s", param);
		result = handle_delete(r, param);
	} else if(streq(type, "ls")){
		result = handle_ls(r);
	} else if(streq(type, "cd")){
		if(!param)	return -1;
		log("params: %s", param);
		result = handle_cd(r, param);
	} else if (streq(type, "mkdir")){
		if(!param)	return -1;
		log("params: %s", param);
		result = handle_mkdir(r, param);
	} else if (streq(type, "pwd")){
		result = handle_pwd(r);
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
