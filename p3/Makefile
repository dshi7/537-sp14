#	Makefile for 537-sp14-p3

CC=gcc
CFLAGS=-g

ALL=test

test : test.o mem.o
	$(CC) $(CFLAGS) test.o mem.o -o $(ALL)

test.o : test.c
	$(CC) $(CFLAGS) -c test.c

mem.o	: mem.c
	$(CC) $(CFLAGS) -c mem.c

#mem.o	: mem.c
#	$(CC) $(CFLAGS) -c -fpic mem.c

#libmem.so : mem.o
#	$(CC) $(CFLAGS) -shared -o $@ $<

#test : test.o libmem.so
#	$(CC) $(CFLAGS) $< -o $@ -lmem -L.

clean:
	rm -rf *.o test *.so

