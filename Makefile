CC = gcc
CFLAGS = -g -Wall -I.
LIBS = -lpthread

threadTest: threadTest.c
	${CC} ${CFLAGS} -c threadTest.c ${LIBS}
	${CC} ${CFLAGS} -o threadTest threadTest.o ${LIBS}

clean:
	rm -f *.o threadTest
