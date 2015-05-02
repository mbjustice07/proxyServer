// Alejandro Nicolette and Max Justice

#include "protocol.h"

//Packet Processing functions
void processIncomingPacket(void *packet, int targetSock);
void processOutgoingPacket(void *packet, int targetSock);

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
  char send_buffer[IP_MAXPACKET];
  char recv_buffer[IP_MAXPACKET];
  memset(recv_buffer, 0, IP_MAXPACKET);
  memset(send_buffer, 0, IP_MAXPACKET);

  struct timeval zeroTime;//Used for polling sockets without waiting
  zeroTime.tv_sec = 0;
  zeroTime.tv_usec = 0;

  // Set up timeval struct for something
  struct timeval readTv;//Sockets will be checked at 1 second intervals
  readTv.tv_sec = 1;
  readTv.tv_usec = 0;
  int n;

  //****************************************** beginning of connection to localhost for telnet

  // we have a session from cproxy.c
    // establish a connection with localhost upon accepting connection from cproxy

    //Create socket
    localSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(localSockfd < 0){
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

  while(1){

      int heartBeatsMissed = 0;
      //********************************************** Connect to CPROXY
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

      while(1){
          //Set up the fd sets
          fd_set readfds;
          fd_set writefds;
          //clear the sets
          FD_ZERO(&readfds);
          FD_ZERO(&writefds);
          //add descriptors to the sets
          FD_SET(proxSession, &readfds);
          FD_SET(localSockfd, &readfds);

          FD_SET(proxSession, &writefds);
          FD_SET(localSockfd, &writefds);

          // this is the n parameter for dealing with the select function
          if (proxSession >= localSockfd){
              n = proxSession + 1;
          }
          else{
              n = localSockfd + 1;
          }
          DB("session = %d\n serverSock = %d\n n = %d\n", proxSession, localSockfd, n);

          int rv;

          rv = select(n, &readfds, NULL, NULL, &readTv);
          readTv.tv_sec = 1;
          readTv.tv_usec = 0;
          if(rv < 0){//Error
              fprintf(stderr, "Error when selecting for received packets, rv = %d\n", rv);
              exit(1);
          }
          else if(rv == 0){//Nothing received
              heartBeatsMissed++;

              DB("Nothing received, heartbeat missed. Missed heartbeat count = %d\n", heartbeatsMissed);

              //TODO handle heartbeat logic here
          }
          else{//At least one socket has a packet to receive
              DB("Something received\n");
              int recStat;
              int innerRv;
              int localSockfd;
              while(FD_ISSET(proxSession, &readfds) || FD_ISSET(localSockfd, &readfds)){
                  if(FD_ISSET(proxSession, &readfds)){//Received something from the client to send to the server
                      //Handle client packets here
                      DB("Packet waiting in localhost\n");

                      recStat = recv(proxSession,recv_buffer , IP_MAXPACKET, 0);
                      if(recStat < 0){
                          fprintf(stderr, "Error when receiving on localSock, status = %d\n", recStat);
                          exit(1);
                      }

                      processOutgoingPacket((void *)recv_buffer, localSockfd);

                  }
                  if(FD_ISSET(localSockfd, &readfds)){//Received something from the server. May be a heartbeat or something to pass to the client.
                      DB("Packet waiting in serverSock\n");

                      recStat = recv(localSockfd, send_buffer, IP_MAXPACKET, 0);
                      if(recStat < 0){
                          fprintf(stderr, "Error when receiving on serverSock, status = %d\n", recStat);
                          exit(1);
                      }

                      processIncomingPacket((void *)send_buffer, proxSession);

                  }
                  //Do another select (without blocking) to see if additional packets are waiting
                  innerRv = select(n, &readfds, NULL, NULL, &zeroTime);
                  if(innerRv < 0){//Error
                      fprintf(stderr, "Error when calling select() in inner loop. Error code = %d\n", innerRv);
                      exit(1);
                  }

          }
/*
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
         memset (recv_bnuffer, 0, sizeof (char) * 256);
         // receive message from server
         localHost_Received = recv (localSockfd, recv_buffer, sizeof (recv_buffer), 0);
        if (localHost_Received < 0)
          error("ERROR reading from socket");



        //*******end of handling the local host send and recieve

        // send reply to Client Proxy Server here
        proxBytes_sent = send (proxSession, recv_buffer, sizeof (recv_buffer), 0);
        if (proxBytes_sent < 0)
       error("ERROR writing to socket");
        */
          }
      }// end of inner while loop
    }// end outer while(1)

  free(recv_buffer);
  free(send_buffer);
  close (proxSession);
  close(localSockfd);
  close (proxSockfd);
  return 0;
}

void processOutgoingPacket(void *packet, int targetSock){
    int sendResult = send(targetSock, (char *)packet, IP_MAXPACKET, 0);
    if(sendResult < 0){
        fprintf(stderr, "Error when sending packet in processOutgoingPacket, result = %d\nError Message: %s\n", sendResult, stererror(errno));
        exit(1);
    }
}

void processIncomingPacket(void *packet, int targetSock){
    int sendResult = send(targetSock, (char *)packet, IP_MAXPACKET, 0);
    if(sendResult < 0){
        fprintf(stderr, "Error when sending packet in processIncomingPacket, result = %d\n", sendResult);
        exit(1);
    }
}
