#include "server.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>


int main(int argc, char* argv[]){
	printf("Hello world!\n");
	socket_listen("test");
	return 0;
}
