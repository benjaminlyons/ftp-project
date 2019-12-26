#include "server.h"
#include <stdlib.h>
#include <string.h> 
char * determine_request_path(const char *file){
	char path[4096] = {0};
	char slash[2]= "/\0";
	strcat(path, RootPath);
	strcat(path, slash);
	strcat(path, file);

	debug("Prior path = %s",  path);
	
	char* real_path = realpath(path, NULL);
	debug("PATH = %s", real_path);
	if(real_path && strncmp(real_path, RootPath, strlen(RootPath)) != 0){
		debug("Requested file not found");
		free(real_path);
		return NULL;
	}
	return real_path;
}
