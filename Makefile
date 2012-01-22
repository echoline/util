ALL=sock mixfifos prefixer toc2lnet oscarsock stdsrv
INSTDIR=bin/

all: ${ALL}

install: all
	mv ${ALL} ${INSTDIR}

oscarsock: oscarsock.o comms.o

sock: sock.o comms.o
	gcc -o sock sock.o comms.o

sock.o: sock.c
	gcc -c sock.c --std=c99

stdsrv: stdsrv.o comms.o
	gcc -o stdsrv stdsrv.o comms.o

stdsrv.o: stdsrv.c
	gcc -c stdsrv.c --std=c99

comms.o: comms.c

clean:
	rm -f ${ALL} *.o
