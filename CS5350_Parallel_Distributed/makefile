OBJS	= matrix_util.o CS5350_Parallel_Distributed.cpp.o
SOURCE	= matrix_util.cpp CS5350_Parallel_Distributed.cpp.cpp
HEADER	= matrix_util.h CS5350_Parallel_Distributed.cpp.h
OUT	= program.out
FLAGS	= -g -c -Wall -fopenmp
LFLAGS	= 
CC	= g++

all:	main.out

program.out: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

matrix_util.o: matrix_util.cpp
	$(CC) $(FLAGS) matrix_util.cpp 

main.o: main.cpp
	$(CC) $(FLAGS) CS5350_Parallel_Distributed.cpp

clean:
	rm -f $(OBJS) $(OUT)
