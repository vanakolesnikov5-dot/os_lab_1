CC=gcc
CFLAGS=-I.

.PHONY: all clean

all: parallel_min_max process_memory parallel_sum

parallel_min_max : parallel_min_max.c utils.c find_min_max.c utils.h find_min_max.h
	$(CC) -o parallel_min_max parallel_min_max.c utils.c find_min_max.c $(CFLAGS)

process_memory : process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

parallel_sum : parallel_sum.c sum_lib.c utils.c sum_lib.h utils.h
	$(CC) -o parallel_sum parallel_sum.c sum_lib.c utils.c $(CFLAGS) -lpthread

clean :
	rm -f parallel_min_max process_memory parallel_sum *.o