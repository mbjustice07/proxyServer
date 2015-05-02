//Alejandro Nicolette and Maxwell Justice
//CSC 425 Project 3

#include "protocol.h"

//localhost and server sockets
int localSock;
int serverSock;

//Packet Processing functions
void processIncomingPacket(void *packet, int targetSock);
void processOutgoingPacket(void *packet, int targetSock);

int main(int argc, char *argv[]){
	//Make sure an argument was given
	if(argc < 2){
		fprintf(stderr, "Usage: %s [Server IP Address]\n", argv[0]);
		exit(1);
	}
	//First establish a connection with localhost
	
	//Create socket
	localSock = socket(AF_INET, SOCK_STREAM, 0);
	if(localSock < 0){
		fprintf(stderr, "Unable to create socket to listen to localhost\n");
		exit(1);
	}

	//Bind the socket to localhost
	struct sockaddr_in local_addr;
	memset(&local_addr, 0 , sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
	local_addr.sin_port = htons(CLIENT_PORT);

	int bindStatus = bind(localSock, (struct sockaddr *)&local_addr, sizeof(local_addr)); 	
	if(bindStatus != 0){
		fprintf(stderr, "Failed to bind to local socket with status %d\n", bindStatus);
		exit(1);
	}
	
	//Listen
	listen(localSock, 5);

	//Accept connection
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	int session = accept(localSock, (struct sockaddr *)&cli_addr, &clilen);
	if(session < 0){
		fprintf(stderr, "accept() failed to establish session when listening to localhost, with session status %d\n", session);
		exit(1);
	}

	DB("Established connection to localhost\n");
	
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

	/*
	Connect to the server in one while loop and perform select operations, acknowledgement handling, and heartbeat counting in an inner loop.
	If the connection to the server is broken, break out of the inner loop and connect again in the outer loop.
	*/
	while(1){
		//Create server socket
		serverSock = socket(AF_INET, SOCK_STREAM, 0);	
		if(serverSock < 0){
			fprintf(stderr, "Unable to create socket to connect to server\n");
			exit(1);
		}

		//Connect socket to server's address
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0 , sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
		uint16_t portnum = (uint16_t)SERVER_PORT;
		serv_addr.sin_port  = htons(portnum);

		//Connect to the server
		int conStatus = connect(serverSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        	if(conStatus < 0){
			fprintf(stderr, "Connection to the server failed with connection status %d\n%s\n", conStatus, strerror(errno));
			exit(1);
		}

		DB("Established connection to the server at %s port %d\n", argv[1], SERVER_PORT);

		
		DB("About to enter inner loop to send/receive packets\n");
/*
		DB("Temp test receive\n");
		
		int testRecStat = recv(session, recBuff, IP_MAXPACKET, 0);
		if(testRecStat < 0){
			fprintf(stderr, "Error when receiving on localSock, status = %d\n%s\n", testRecStat, strerror(errno));
			exit(1);
		}
		DB("TEST: Just received from localhost with status = %d\n", testRecStat);
*/
		//Keep count of missed heartbeats
		int heartbeatsMissed = 0;	
		//Inner loop
		while(1){
			//Set up the fd sets 
			fd_set readfds;
			fd_set writefds;
			//clear the sets
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			//add descriptors to the sets
			FD_SET(session, &readfds);
			FD_SET(serverSock, &readfds);

			FD_SET(session, &writefds);
			FD_SET(serverSock, &writefds);

			//Set the n param
			if(session >= serverSock){
				n = session + 1;
			}
			else{
				n = serverSock + 1;
			}
			DB("session = %d\n serverSock = %d\n n = %d\n", session, serverSock, n);
	
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
				int recStat;
				int innerRv;
				while(FD_ISSET(session, &readfds) || FD_ISSET(serverSock, &readfds)){
					if(FD_ISSET(session, &readfds)){//Received something from the client to send to the server
						//Handle client packets here
						DB("Packet waiting in localhost\n");
					
						recStat = recv(session, recBuff, IP_MAXPACKET, 0);
						if(recStat < 0){
							fprintf(stderr, "Error when receiving on localSock, status = %d\nError message: %s\n", recStat, strerror(errno));
							exit(1);
						}
						DB("Received packet from localhost\n");						

						processOutgoingPacket((void *)recBuff, serverSock);
	
					}
					if(FD_ISSET(serverSock, &readfds)){//Received something from the server. May be a heartbeat or something to pass to the client.
						DB("Packet waiting in serverSock\n");

						recStat = recv(serverSock, sendBuff, IP_MAXPACKET, 0);	
						if(recStat < 0){
							fprintf(stderr, "Error when receiving on serversock, status = %d\nError message: %s\n", recStat, strerror(errno));
							exit(1);
						}
						DB("Received packet from server\n");			
			
						processIncomingPacket((void *)sendBuff, session);

					}
		
					//Reset fds and do a non-blocking select to see if additional packets are waiting	
					//clear the sets
					FD_ZERO(&readfds);
					FD_ZERO(&writefds);
					//add descriptors to the sets
					FD_SET(session, &readfds);
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

void processOutgoingPacket(void *packet, int targetSock){
	int sendResult = send(targetSock, (char *)packet, IP_MAXPACKET, 0);
	if(sendResult < 0){
		fprintf(stderr, "Error when sending packet in processOutgoingPacket, result = %d\n", sendResult);
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
