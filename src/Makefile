CC = g++
CFLAGS = -std=c++11 -g -rdynamic -Wall

OBJS = move.o bitboards.o position.o movegen.o
TARGETS = tester

tester: $(OBJS) testing.cpp
	$(CC) $(CFLAGS) $(OBJS) testing.cpp -o tester

position.o:  position.cpp position.h
	$(CC) $(CFLAGS) -c position.cpp -o position.o

bitboards.o: bitboards.cpp bitboards.h
	$(CC) $(CFLAGS) -c bitboards.cpp -o bitboards.o 

movegen.o: movegen.cpp movegen.h
	$(CC) $(CFLAGS) -c movegen.cpp -o movegen.o

move.o : move.cpp move.h
	$(CC) $(CFLAGS) -c move.cpp -o move.o

clean :
	rm -f *~ \#*# $(OBJS) $(TARGETS)
