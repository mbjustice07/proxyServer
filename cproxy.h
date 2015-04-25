//Alejandro Nicolette and Maxwell Justice
#include <assert.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>

#include "linkedList.h"

#define CLIENT_PORT 5200
#define LOCALHOST "127.0.0.1"

#define SERVER_PORT 620

#define HEARTBEAT_LIMIT 3

#ifdef DEBUG
#define DB(...) fprintf(stderr, __VA_ARGS__)
#else
#define DB(...) ((void)0)
#endif
