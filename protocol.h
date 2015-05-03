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
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


#include "linkedList.h"

#define CLIENT_PORT 5200
#define LOCALHOST "127.0.0.1"

#define SERVER_PORT 6200

#define SPROXY_LOCAL 23

#define HEARTBEAT_LIMIT 3

#ifdef DEBUG
#define DB(...) fprintf(stderr, __VA_ARGS__)
#else
#define DB(...) ((void)0)
#endif


#ifdef _LINUX_
#include <stdint.h>
#endif /* _LINUX_ */

#include <sys/types.h>
#include <arpa/inet.h>

#ifndef PX_HEART
#define PX_HEART        0x0202
#endif

#ifndef PX_NEWCON
#define PX_NEWCON       0x0404
#endif

#ifndef PX_DAT
#define PX_DAT          0x0303
#endif

#ifndef IP_MAXPACKET
#define IP_MAXPACKET 1500
#endif

typedef struct px
{
        uint32_t px_newaddr;    // the new connection initiation
        uint16_t px_tos;         // type of service
        uint16_t px_len;        // total length of packet
        uint32_t px_ack;        // acknowlegdement numbers
        uint32_t px_seq;        // sequence number 
        uint32_t px_ses;        // session number for extra credit
} px;

