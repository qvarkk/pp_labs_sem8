CC = g++
CFLAGS = -fopenmp -O0

all: exec

exec: main.o
	$(CC) $(CFLAGS) main.o -o exec

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o exec
