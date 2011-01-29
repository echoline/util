ALL="sock"

all: ${ALL}

sock: sock.o comms.o
	gcc -o sock sock.o comms.o

sock.o: sock.c
	gcc -c sock.c --std=c99

comms.o: comms.c

clean:
	rm -f ${ALL} *.o
