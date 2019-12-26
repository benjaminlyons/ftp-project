// this is loosely based off dr. bui's spidey project

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <unistd.h>

/* use the logging stuff from dr. bui's code */
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define fatal(M, ...) fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE)
#define log(M, ...)	fprintf(stderr, "[%5d] LOG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

int socket_listen(const char *port);
