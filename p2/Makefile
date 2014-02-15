#	Makefile for 537-sp14-p2

OBJS = parser.o basic.o shell.o
CC=gcc
CFLAGS=-Wall 

ALL=mysh

mysh : shell.o parser.o basic.o
	$(CC) $(CFLAGS) $(OBJS) -o $(ALL)

parser.o :
	$(CC) $(CFLAGS) -c parser.c

basic.o	:
	$(CC) $(CFLAGS) -c basic.c

shell.o : 
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -rf *o mysh
