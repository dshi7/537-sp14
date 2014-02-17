#	Makefile for 537-sp14-p2

OBJS = basic.o parser.o single.o shell.o
CC=gcc
CFLAGS=-Wall 

ALL=mysh

mysh : basic.o parser.o single.o shell.o
	$(CC) $(CFLAGS) $(OBJS) -o $(ALL)

basic.o	:
	$(CC) $(CFLAGS) -c basic.c

parser.o :
	$(CC) $(CFLAGS) -c parser.c

single.o :
	$(CC) $(CFLAGS) -c single.c

shell.o : 
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -rf *o mysh

