ALL=choose
LDFLAGS=-static
INSTDIR=bin/

all: ${ALL}

install: all
	mv ${ALL} ${INSTDIR}

oscarsock: oscarsock.o comms.o

sock: sock.o comms.o
	gcc -o sock sock.o comms.o

sock.o: sock.c
	gcc -c sock.c --std=c99

filesrv: filesrv.o comms.o
	gcc -o filesrv filesrv.o comms.o

stdsrv: stdsrv.o comms.o
	gcc -o stdsrv stdsrv.o comms.o

stdsrv.o: stdsrv.c
	gcc -c stdsrv.c --std=c99

comms.o: comms.c

clean:
	rm -f ${ALL} *.o
