CC = gcc
CFLAGS = -g -Wall -I.
CFLAGS_D = -g -Wall -I. -DDEBUG=1
LIBS = -lpthread

all: cproxy sproxy

cproxy: cproxy.c
	${CC} ${CFLAGS} -c cproxy.c ${LIBS}
	${CC} ${CFLAGS} -c linkedList.c ${LIBS}
	${CC} ${CFLAGS} -o cproxy cproxy.o linkedList.o ${LIBS}

cproxy_D: cproxy.c
	${CC} ${CFLAGS_D} -c cproxy.c ${LIBS}
	${CC} ${CFLAGS_D} -c linkedList.c ${LIBS}	
	${CC} ${CFLAGS_D} -o cproxy cproxy.o linkedList.o ${LIBS}

sproxy: sproxy.c
	${CC} ${CFLAGS} -c sproxy.c ${LIBS}
	${CC} ${CFLAGS} -c linkedList.c ${LIBS}
	${CC} ${CFLAGS} -o sproxy sproxy.o linkedList.o ${LIBS}

sproxy_D: sproxy.c
	${CC} ${CFLAGS_D} -c sproxy.c ${LIBS}
	${CC} ${CFLAGS_D} -c linkedList.c ${LIBS}	
	${CC} ${CFLAGS_D} -o sproxy sproxy.o linkedList.o ${LIBS}

clean:
	rm -f *.o core cproxy sproxy


