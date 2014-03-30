C=gcc
CFLAGS=-g

all : test

spin.o : spin.c
		$(CC) $(CFLAGS) -c -fpic spin.c

counter.o : counter.c
		$(CC) $(CFLAGS) -c -fpic counter.c
		        
list.o : list.c
		$(CC) $(CFLAGS) -c -fpic list.c
		        
hash.o : hash.c
		$(CC) $(CFLAGS) -c -fpic hash.c
		        
libspin.so: spin.o
		$(CC) $(CFLAGS) -shared -o $@ $<

libcounter.so : counter.o
		$(CC) $(CFLAGS) -shared -o $@ $<

liblist.so : list.o
		$(CC) $(CFLAGS) -shared -o $@ $<

libhash.so : hash.o
		$(CC) $(CFLAGS) -shared -o $@ $<

test : test.o libhash.so liblist.so libcounter.so libspin.so
		$(CC) $(CFLAGS) $< -o $@ -lhash -llist -lcounter -lspin -L. -lpthread


clean :
		rm -rf *.o test *.so
