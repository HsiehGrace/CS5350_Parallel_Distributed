OBJS	= matrix_util.o CS5350_Parallel_Distributed.o
SOURCE	= matrix_util.cpp CS5350_Parallel_Distributed.cpp
HEADER	= matrix_util.h CS5350_Parallel_Distributed.h
OUT	= a.out
FLAGS	= -g -c -Wall -fopenmp -Iinclude
LFLAGS	= 
CC	= g++

all:	a.out

a.out: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

matrix_util.o: matrix_util.cpp
	$(CC) $(FLAGS) matrix_util.cpp 

CS5350_Parallel_Distributed.o: CS5350_Parallel_Distributed.cpp
	$(CC) $(FLAGS) CS5350_Parallel_Distributed.cpp 

clean:
	rm -f $(OBJS) $(OUT)
