CC = gcc
CFLAGS =

all: exec

exec: main.o
	$(CC) main.o -o exec

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

O0: CFLAGS = -O0
O0: clean exec

O2: CFLAGS = -O2
O2: clean exec

clean:
	rm -rf *.o exec
