CC      = mpic++
CFLAGS  = -O2 -std=c++17 -Wall -Wextra

TARGET = main

all: $(TARGET)

$(TARGET): main.cpp
	$(CC) $(CFLAGS) main.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)

run1: $(TARGET)
	mpiexec -n 1 --oversubscribe ./$(TARGET)

run2: $(TARGET)
	mpiexec -n 2 --oversubscribe ./$(TARGET)

run3: $(TARGET)
	mpiexec -n 3 --oversubscribe ./$(TARGET)

run4: $(TARGET)
	mpiexec -n 4 --oversubscribe ./$(TARGET)

run5: $(TARGET)
	mpiexec -n 5 --oversubscribe ./$(TARGET)

run6: $(TARGET)
	mpiexec -n 6 --oversubscribe ./$(TARGET)