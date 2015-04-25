#include "threadTest.h"

int stopPrinting = 0;
pthread_mutex_t stopMutex;

void* printLower(void *);
void setStop(int n);
int getStop();
/*
Simple thread demo. main prints each letter of the alphabet in uppcase, waiting 2 seconds between each letter. pLowerThread prints each letter in lower case, waiting only 1 second between each letter.
When main finishes printing, it sets a flag protected by mutexes to tell pLowerThread to finish, then it joins and the program exits.
*/
int main(int argc, char *argv[]){
	int i;
	char ch = 'A';
	pthread_t pLowerThread;

	int threadStatus = pthread_create(&pLowerThread, NULL, printLower, NULL);
	if(threadStatus){
		fprintf(stderr, "Failed to create thread: status = %d\n", threadStatus);
		exit(1);
	}

	for(i = 0; i < 26; i++){
		sleep(2);
		printf("%c\n", ch++);
	}
	printf("Upper is finished\n");
	setStop(1);
	
	threadStatus = pthread_join(pLowerThread, NULL);
	if(threadStatus){
		fprintf(stderr, "Failed to joing with thread: status = %d\n", threadStatus);
		exit(1);
	}

	return 0;	
}

void* printLower(void * arg){
	int i;
	while(1){
		char ch = 'a';
		for(i = 0; i < 26; i++){
			sleep(1);	
			if(getStop()){
				goto done;
			}
			printf("%c\n", ch++);
		}
	}	
done:
	printf("Stop signal received by printLower\n");
	return NULL;
}

void setStop(int n){
	pthread_mutex_lock(&stopMutex);
	stopPrinting = n;
	pthread_mutex_unlock(&stopMutex);
}

int getStop(){
	int n;
	pthread_mutex_lock(&stopMutex);
	n = stopPrinting;
	pthread_mutex_unlock(&stopMutex);
	return n;
}

