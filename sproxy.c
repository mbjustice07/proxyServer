//Alejandro Nicolette and Maxwell Justice
//CSC 425 Project 3

#include "protocol.h"

//localhost and server sockets
int clientSock;
int serverSock;

//Packet Processing functions
void processIncomingPacket(void *packet, int targetSock, int length);
void processOutgoingPacket(void *packet, int targetSock, int length);

int main(int argc, char *argv[]){
	//First establish a connection with cproxy
	
	//Create socket
	clientSock = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSock < 0){
		fprintf(stderr, "Unable to create socket to listen to localhost\n");
		exit(1);
	}

	//Bind the socket 
	struct sockaddr_in client_addr;
	memset(&client_addr, 0 , sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(SERVER_PORT);

	int bindStatus = bind(clientSock, (struct sockaddr *)&client_addr, sizeof(client_addr)); 	
	if(bindStatus != 0){
		fprintf(stderr, "Failed to bind to local socket with status %d\n", bindStatus);
		exit(1);
	}
	
	//Listen
	listen(clientSock, 5);

	//Accept connection
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	int clientSession = accept(clientSock, (struct sockaddr *)&cli_addr, &clilen);
	if(clientSession < 0){
		fprintf(stderr, "accept() failed to establish clientSession when listening to localhost, with clientSession status %d\n", clientSession);
		exit(1);
	}

	DB("Established connection to the client\n");
	
	//Set up timeval struct
	struct timeval readTv;//Sockets will be checked at 1 second intervals
	readTv.tv_sec = 1;
	readTv.tv_usec = 0;
	int n;

	struct timeval zeroTime;//Used for polling sockets without waiting
	zeroTime.tv_sec = 0;
	zeroTime.tv_usec = 0;

	//Set up buffers to hold packets for sending and receiving
	char recBuff[IP_MAXPACKET];
	char sendBuff[IP_MAXPACKET];
	memset(recBuff, 0, IP_MAXPACKET);
	memset(sendBuff, 0, IP_MAXPACKET);
    
    //Connect to the telnet daemon
    
    //Create server socket
	serverSock = socket(AF_INET, SOCK_STREAM, 0);	
	if(serverSock < 0){
		fprintf(stderr, "Unable to create socket to connect to telnet daemon\n");
		exit(1);
	}

	//Connect socket to localhost port 23
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0 , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, LOCALHOST, &serv_addr.sin_addr);
	uint16_t portnum = (uint16_t)SPROXY_LOCAL;
	serv_addr.sin_port  = htons(portnum);

	//Connect to the server
	int conStatus = connect(serverSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
       	if(conStatus < 0){
		fprintf(stderr, "Connection to the server failed with connection status %d\n%s\n", conStatus, strerror(errno));
		exit(1);
	}
    

	/*
        Pass packets between server and client the inner while loop. If the connection is broken, break out of the inner while loop and accept a new connection.
	*/
    int connectionBroken = 0;
	while(1){

        //If the connection has been broken, accept a new one
        if(connectionBroken){
            DB("Connection broken, accepting new connection\n");       
           	//Create socket
            clientSock = socket(AF_INET, SOCK_STREAM, 0);
            if(clientSock < 0){
                fprintf(stderr, "Unable to create socket to listen to localhost\n");
                exit(1);
            }
            //Bind the socket 
            struct sockaddr_in client_addr;
            memset(&client_addr, 0 , sizeof(client_addr));
            client_addr.sin_family = AF_INET;
            client_addr.sin_addr.s_addr = INADDR_ANY;
            client_addr.sin_port = htons(SERVER_PORT);
            int bindStatus = bind(clientSock, (struct sockaddr *)&client_addr, sizeof(client_addr)); 	
            if(bindStatus != 0){
                fprintf(stderr, "Failed to bind to local socket with status %d\n", bindStatus);
                exit(1);
            }	
            //Listen
            listen(clientSock, 5);
            //Accept connection
            struct sockaddr_in cli_addr;
            socklen_t clilen;
            int clientSession = accept(clientSock, (struct sockaddr *)&cli_addr, &clilen);
            if(clientSession < 0){
                fprintf(stderr, "accept() failed to establish clientSession when listening to localhost, with clientSession status %d\n", clientSession);
                exit(1);
            }       
        }
        
		DB("About to enter inner loop to send/receive packets\n");
		//Keep count of missed heartbeats
		int heartbeatsMissed = 0;	
		//Inner loop
		while(!connectionBroken){
			//Set up the fd sets 
			fd_set readfds;
			fd_set writefds;
			//clear the sets
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			//add descriptors to the sets
			FD_SET(clientSession, &readfds);
			FD_SET(serverSock, &readfds);

			FD_SET(clientSession, &writefds);
			FD_SET(serverSock, &writefds);

			//Set the n param
			if(clientSession >= serverSock){
				n = clientSession + 1;
			}
			else{
				n = serverSock + 1;
			}
			DB("clientSession = %d\n serverSock = %d\n n = %d\n", clientSession, serverSock, n);
	
			//Return value of select
			int rv;

			//Check for packets to receive
			rv = select(n, &readfds, NULL, NULL, &readTv);
			//Reset timeval
			readTv.tv_sec = 1;
			readTv.tv_usec = 0;
			if(rv < 0){//Error
				fprintf(stderr, "Error when selecting for received packets, rv = %d\n", rv);
				exit(1);
			}
			else if(rv == 0){//Nothing received
				heartbeatsMissed++;

				DB("Nothing received, heartbeat missed. Missed heartbeat count = %d\n", heartbeatsMissed);

				//TODO handle heartbeat logic here
			}
			else{//At least one socket has a packet to receive
				DB("Something received\n");
				int recLen;
				int innerRv;
				while(FD_ISSET(clientSession, &readfds) || FD_ISSET(serverSock, &readfds)){
					if(FD_ISSET(clientSession, &readfds)){//Received something from the client to send to the server
						//Handle client packets here
						DB("Packet waiting in localhost\n");
					
						recLen = recv(clientSession, recBuff, IP_MAXPACKET, 0);
						if(recLen < 0){
							fprintf(stderr, "Error when receiving on clientSock, status = %d\nError message: %s\n", recLen, strerror(errno));
							exit(1);
						}
						DB("Received packet from localhost\n");						

						processOutgoingPacket((void *)recBuff, serverSock, recLen);
	
					}
					if(FD_ISSET(serverSock, &readfds)){//Received something from the server. May be a heartbeat or something to pass to the client.
						DB("Packet waiting in serverSock\n");

						recLen = recv(serverSock, sendBuff, IP_MAXPACKET, 0);	
						if(recLen < 0){
							fprintf(stderr, "Error when receiving on serversock, status = %d\nError message: %s\n", recLen, strerror(errno));
							exit(1);
						}
						DB("Received packet from server\n");			
			
						processIncomingPacket((void *)sendBuff, clientSession, recLen);

					}
		
					//Reset fds and do a non-blocking select to see if additional packets are waiting	
					//clear the sets
					FD_ZERO(&readfds);
					FD_ZERO(&writefds);
					//add descriptors to the sets
					FD_SET(clientSession, &readfds);
					FD_SET(serverSock, &readfds);

					innerRv = select(n, &readfds, NULL, NULL, &zeroTime);
					if(innerRv < 0){//Error
						fprintf(stderr, "Error when calling select() in inner loop. Return value = %d\n", innerRv);
						exit(1);
					}

				}
			}

		}//End inner while

		
	}//while

	return 0;
}

void processOutgoingPacket(void *packet, int targetSock, int length){
	int sendResult = send(targetSock, (char *)packet, length, 0);
	if(sendResult < 0){
		fprintf(stderr, "Error when sending packet in processOutgoingPacket, result = %d\n", sendResult);
		exit(1);
	}
	DB("Sent packet to server\n");
}

void processIncomingPacket(void *packet, int targetSock, int length){
	int sendResult = send(targetSock, (char *)packet, length, 0);
	if(sendResult < 0){
		fprintf(stderr, "Error when sending packet in processIncomingPacket, result = %d\n", sendResult);
		exit(1);
	}
	DB("Sent packet to localhost\n");
}
