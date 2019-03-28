CFLAGS ?=	-Wall -ggdb -W -O
CC ?=		gcc
LIBS ?=
LDFLAGS ?=
PREFIX ?=	/usr/local
VERSION = 1.5
TMPDIR = /tmp/webbench-$(VERSION)

all:   webbench tags

tags:  *.c
	-ctags *.c

install: webbench
	install -s webbench $(DESTDIR)$(PREFIX)/bin	
	install -m 644 webbench.1 $(DESTDIR)$(PREFIX)/man/man1	
	install -d $(DESTDIR)$(PREFIX)/share/doc/webbench

webbench: webbench.o socket.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o webbench webbench.o socket.o $(LIBS) 

webbench.o: webbench.c
	$(CC) -g -c -Wall webbench.c $(LIBS)

socket.o: socket.c
	$(CC) -g -c -Wall socket.c $(LIBS)

clean:
	-rm -f *.o webbench *~ core *.core tags
	
tar:   clean
	rm -rf $(TMPDIR)
	install -d $(TMPDIR)
	cp -p Makefile webbench.c socket.c webbench.1 $(TMPDIR)
	-cd $(TMPDIR) && cd .. && tar cozf webbench-$(VERSION).tar.gz webbench-$(VERSION)

.PHONY: clean install all tar
