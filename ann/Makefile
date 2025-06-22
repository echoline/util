CFLAGS=-fopenmp

ann: annio.o ann.o main.o
	cc -o ann annio.o ann.o main.o -fopenmp

clean:
	rm -rf *.o ann

