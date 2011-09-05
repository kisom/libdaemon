# BASEMENT MAKEFILE
CFLAGS := -Wall -g
CC=gcc
LFLAGS := -fPIC
LDFLAGS := -L.


all: libdaemon.a

install: libdaemon.a
	install -m 0755 -o root -g bin libdaemon.a /usr/local/lib
	install -m 0755 -o root -g bin daemon.h /usr/local/include

uninstall: 
	rm -f /usr/local/lib/libdaemon.a
	rm -f /usr/local/include/daemon.h

libdaemon.a: daemon.c
	$(CC) $(CFLAGS) $(LFLAGS) -o libdaemon.o -c daemon.c
	ar r libdaemon.a libdaemon.o
	rm libdaemon.o

testd: testd.c
	$(CC) $(CFLAGS) -c -o testd testd.c -ldaemon

clean:
	rm -f *.o
	rm -f *.a
	rm -f testd

.PHONY: clean uninstall install all
