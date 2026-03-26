CC      = mpicc
CFLAGS  = -O2

SEQ_CC      = gcc
SEQ_CFLAGS  = -O2

TARGET_MPI = dot_mpi
TARGET_SEQ = dot_seq

all: $(TARGET_MPI) $(TARGET_SEQ)

$(TARGET_MPI): dot_mpi.c
	$(CC) $(CFLAGS) dot_mpi.c -o $(TARGET_MPI)

$(TARGET_SEQ): dot_seq.c
	$(SEQ_CC) $(SEQ_CFLAGS) dot_seq.c -o $(TARGET_SEQ)

clean:
	rm -f $(TARGET_MPI) $(TARGET_SEQ)

run1: $(TARGET_MPI)
	mpiexec -n 1 --oversubscribe ./$(TARGET_MPI)

run2: $(TARGET_MPI)
	mpiexec -n 2 --oversubscribe ./$(TARGET_MPI)

run4: $(TARGET_MPI)
	mpiexec -n 4 --oversubscribe ./$(TARGET_MPI)

run8: $(TARGET_MPI)
	mpiexec -n 8 --oversubscribe ./$(TARGET_MPI)

run-seq: $(TARGET_SEQ)
	./$(TARGET_SEQ)
