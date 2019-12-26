#pragma once

/* use the logging stuff from dr. bui's code */
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define fatal(M, ...) fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE)
#define log(M, ...)	fprintf(stderr, "[%5d] LOG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

#define streq(a, b) (strcmp((a), (b)) == 0)
#define chomp(s) (s)[strlen(s) - 1] = '\0'

#define min(a, b) ( (a) < (b) ? (a) : (b) )
