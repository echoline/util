ALL=sock mixfifos prefixer toc2lnet oscarsock
INSTDIR=bin/

all: ${ALL}

install: all
	mv ${ALL} ${INSTDIR}

oscarsock: oscarsock.o comms.o

sock: sock.o comms.o
	gcc -o sock sock.o comms.o

sock.o: sock.c
	gcc -c sock.c --std=c99

comms.o: comms.c

clean:
	rm -f ${ALL} *.o
