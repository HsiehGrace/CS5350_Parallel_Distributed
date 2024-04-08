OBJS	= matrix_util.o main.o
SOURCE	= matrix_util.cpp main.cpp
HEADER	= matrix_util.h main.h
OUT	= a.out
FLAGS	= -g -c -Wall -fopenmp
LFLAGS	= 
CC	= g++

all:	a.out

a.out: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

matrix_util.o: matrix_util.cpp
	$(CC) $(FLAGS) matrix_util.cpp 

main.o: main.cpp
	$(CC) $(FLAGS) CS5350_Parallel_Distributed.cpp 

clean:
	rm -f $(OBJS) $(OUT)
