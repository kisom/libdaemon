CC=gcc
CFLAGS := -Wall -Werror -g -I/usr/local/include
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

%.o: *.c
	$(CC) $(CFLAGS) $<

tags:
	ctags *.[ch] >tags

secreports: 
	rats *.c > rats.txt
	lint *.c > lint.txt

doc:
	pdflatex -output-directory doc/ libdaemon.latex

clean:
	rm -f *.o
	rm -f *.a
	rm -f rats.txt lint.txt

.PHONY: clean uninstall install all secreports tags doc
