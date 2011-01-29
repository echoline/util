ALL=sock mixfifos prefixer toc2lnet
INSTDIR=bin/

all: ${ALL}

install: all
	mv ${ALL} ${INSTDIR}

sock: sock.o comms.o
	gcc -o sock sock.o comms.o

sock.o: sock.c
	gcc -c sock.c --std=c99

comms.o: comms.c

clean:
	rm -f ${ALL} *.o
