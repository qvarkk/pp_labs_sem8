CC = gcc
CFLAGS = -fopenmp -O0

all: exec

exec: main.o
	$(CC) $(CFLAGS) main.o -o exec

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -rf *.o exec
