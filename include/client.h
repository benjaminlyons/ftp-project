#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <unistd.h>

extern char* Host;
extern char* Port;

int handle_get_command(FILE* socket_stream, char* filename);
int handle_put_command(FILE* socket_stream, char* filename);
