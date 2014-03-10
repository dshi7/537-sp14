#	Makefile for 537-sp14-p3

CC=gcc
CFLAGS=-g 
LDFLAGS=-shared

ALL=prog.exe

prog.exe: mymain.o libmem.so
	$(CC) $(CFLAGS) $< -o $@ -lmem -L.

mem.o	: 
	$(CC) $(CFLAGS) -c -fpic mem.c

mymain.o :
	$(CC) -c mymain.c

libmem.so : mem.o
	$(CC) $(LDFLAGS) -o $@ $<

clean:
	rm -rf *.o *.so *.exe
