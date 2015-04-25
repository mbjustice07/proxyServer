//Alejandro Nicolette and Maxwell Justice
//CSC 425 Project 3

#include "cproxy.h"

//localhost and server sockets
int localSock;
int serverSock;

//Packet Processing functions
int processIncomingPacket(void *packet);
int processOutgoingPacket(void *packet);

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
	

	//Set up the fd set and declare the timeval struct
	fd_set readfds;
	struct timeval readTv;//Sockets will be checked at 1 second intervals
	readTv.tv_sec = 1;
	readTv.tv_usec = 0;
	int n;
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
		if(conStatus != 0){
			fprintf(stderr, "Connection to the server failed with connection status %d\n", conStatus);
			exit(1);
		}

		DB("Established connection to the server at %s port %d\n", argv[1], SERVER_PORT);

		//clear the set
		FD_ZERO(&readfds);
		//add descriptors to the set
		FD_SET(localSock, &readfds);
		FD_SET(serverSock, &readfds);

		//Set the n param
		n = serverSock + 1;

		//Keep count of missed heartbeats
		int heartbeatsMissed = 0;	
	
		//Inner loop
		while(1){

		}//End inner while

		
	}//while

	return 0;
}

