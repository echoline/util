ALL=choose
LDFLAGS=-static -g
CFLAGS=-g
INSTDIR=bin/

all: ${ALL}

install: all
	mv ${ALL} ${INSTDIR}

oscarsock: oscarsock.o comms.o

sock: sock.o comms.o

sock.o: sock.c

filesrv: filesrv.o comms.o
	gcc -o filesrv filesrv.o comms.o

stdsrv: stdsrv.o comms.o
	gcc -o stdsrv stdsrv.o comms.o

stdsrv.o: stdsrv.c

comms.o: comms.c

clean:
	rm -f ${ALL} *.o
