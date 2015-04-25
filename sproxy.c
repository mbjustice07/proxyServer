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
#include "protocol.h"

// localhost and client sockets
int localSock;
int clientSock;

void error(char *msg){
  perror(msg);
  exit(1);
}

int main(int argc, char * argv[]){
  // socket file descriptors, bytes sent and received
  int proxSockfd, proxSession, proxBytes_sent, proxBytes_received;

  // structure that contains info about internet addr
  struct sockaddr_in proxyServ_addr, proxyCli_addr;

  // stores the length of client's sockaddr struct for accept function
  socklen_t proxyClilen;

  //****local host variables
  struct sockaddr_in localServ_addr;
  int localSockfd, localHost_Sent, localHost_Received;

  //****

  // create a buffer
  char* send_buffer = (char*)malloc(IP_MAXPACKET*sizeof(char));
  char* recv_buffer = (char*)malloc(IP_MAXPACKET*sizeof(char));

  // create a stream socket (TCP) for the sproxy
  proxSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (proxSockfd < 0)
    error ("ERROR opening socket");
  memset (&proxyServ_addr, 0, sizeof(proxyServ_addr));
  proxyServ_addr.sin_family = AF_INET;
  proxyServ_addr.sin_addr.s_addr = INADDR_ANY;
  proxyServ_addr.sin_port = htons (SERVER_PORT);

  // bind socket to an address
  if (bind(proxSockfd, (struct sockaddr *) &proxyServ_addr, sizeof (proxyServ_addr)) < 0)
    error("ERROR on binding");

  // listen to socket, set maximum pending connection is 5
  listen (proxSockfd, 5);

  // set length of client address
  proxyClilen = sizeof (proxyCli_addr);
  // accept first incoming connection on the pending queue, returns a new file descriptor
  proxSession = accept (proxSockfd, (struct sockaddr *) &proxyCli_addr, &proxyClilen);

  if (proxSession < 0){
    error("ERROR on accept");
    exit(1);
  }
  //****************************************** beginning of connection to localhost for telnet
   // we have a session from cproxy.c
      // establish a connection with localhost upon accepting connection from cproxy

      //Create socket
      localSock = socket(AF_INET, SOCK_STREAM, 0);
      if(localSock < 0){
          fprintf(stderr, "Unable to create socket to listen to localhost\n");
          exit(1);
      }

      // 0 out the struct
      memset (&localServ_addr, 0, sizeof (localServ_addr));
      // domain is Internet
      localServ_addr.sin_family = AF_INET;

      // copy convert and copy server's ip addr into serv_addr
      if(inet_pton(AF_INET, LOCALHOST, &localServ_addr.sin_addr) < 0)
        error ("ERROR, copying ip\n");
      // set server's port number, stores it in network byte order
      localServ_addr.sin_port = htons (SPROXY_LOCAL);
      // connect to server
      if (connect(localSockfd,(struct sockaddr *) &localServ_addr, sizeof (localServ_addr)) < 0)
        error ("ERROR connecting to local host");
    //******************************************************* end of localhost connection for telnet

  // 0 out the recieve buffer
  memset (send_buffer, 0, sizeof (send_buffer));
  // receive data from client Proxy  here
  proxBytes_received = recv (proxSession, send_buffer, sizeof (send_buffer), 0);
  if (proxBytes_received < 0)
    error("ERROR reading from Client Proxy Socket");


  //*******handling the localhost send and receive

  localHost_Sent = send (localSockfd, send_buffer, sizeof (send_buffer), 0);

  if (localHost_Sent < 0)
    error ("ERROR writing to socket");
  // 0 out receive buffer
  memset (recv_buffer, 0, sizeof (char) * 256);
  // receive message from server
  localHost_Received = recv (localSockfd, recv_buffer, sizeof (recv_buffer), 0);
  if (localHost_Received < 0)
    error("ERROR reading from socket");



  //*******end of handling the local host send and recieve

  // send reply to Client Proxy Server here
  proxBytes_sent = send (proxSession, recv_buffer, sizeof (recv_buffer), 0);
  if (proxBytes_sent < 0)
    error("ERROR writing to socket");

  free(recv_buffer);
  free(send_buffer);
  close (proxSession);
  close(localSockfd);
  close (proxSockfd);
  return 0;
}
