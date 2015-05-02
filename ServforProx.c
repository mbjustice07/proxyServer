/*
  This is a simple server program that uses stream sockets (TCP). It listens on port
  8888, receives 1 short message from a client, then replies to client and exits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

/*
 Prints error message and exits.
 */
void error(char *msg){
  perror(msg);
  exit(1);
}

int main(int argc, char * argv[]){
  // port number to listen on
  int portnum = 8888;
  // socket file descriptors, bytes sent and received
  int proxSockfd, proxSession, proxBytes_sent, proxBytes_received;

  // stores the length of client's sockaddr struct for accept function
  socklen_t proxClilen;

  // receive buffer
  char buffer[256];

  // structure that contains info about internet addr
  struct sockaddr_in proxServ_addr, proxCli_addr;

  // server's reply
  char * reply = "Hello client";

  // create a stream socket (TCP)
  proxSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (proxSockfd < 0)
    error ("ERROR opening socket");

  // 0 out the struct
  memset (&proxServ_addr, 0, sizeof(proxServ_addr));
  // domain is Internet
  proxServ_addr.sin_family = AF_INET;
  // don't need a specific address, any of the network interface will do
  proxServ_addr.sin_addr.s_addr = INADDR_ANY;
  // set port number, and stores it in network byte order
  proxServ_addr.sin_port = htons (portnum);

  // bind socket to an address
  if (bind(proxSockfd, (struct sockaddr *) &proxServ_addr, sizeof (proxServ_addr)) < 0)
    error("ERROR on binding");

  // listen to socket, set maximum pending connection is 5
  listen (proxSockfd, 5);
  // set length of client address
  proxClilen = sizeof (proxCli_addr);
  // accept first incoming connection on the pending queue, returns a new file descriptor
  proxSession = accept (proxSockfd, (struct sockaddr *) &proxCli_addr, &proxClilen);

  if (proxSession < 0)
    error("ERROR on accept");
  // 0 out the recieve buffer
  memset (buffer, 0, sizeof (buffer));
  // receive data from client and puts it in buffer
  proxBytes_received = recv (proxSession, buffer, sizeof (buffer), 0);
  if (proxBytes_received < 0)
    error("ERROR reading from socket");

  printf ("Client said: %s\n",buffer);
  // send client a reply
  proxBytes_sent = send (proxSession, reply, sizeof (char) * strlen (reply), 0);
  if (proxBytes_sent < 0)
    error("ERROR writing to socket");

  close (proxSession);
  close (proxSockfd);
  return 0;
}
