PROGRAM=xdibda00

SRC=xdibda00.c

CFLAGS=-std=c11 -Wextra -O -pedantic -Wall -pthread
CC=gcc

all:  clean ${PROGRAM} 

${PROGRAM}: 
	$(CC) $(CFLAGS) $(SRC) -o $@

clean:
	rm -f *.o ${PROGRAM}
