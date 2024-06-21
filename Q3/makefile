.PHONY: all clean

CC = g++
FLAGS = -g -std=c++11

all: mync ttt

mync: mynetcat.o mync_client.o mync_server.o
	$(CC) $(FLAGS) -o $@ $^

ttt: ttt.o tttfunctions.o
	$(CC) $(FLAGS) -o $@ $^

ttt.o: ttt.cpp
	$(CC) $(FLAGS) -c $<

tttfunctions.o: tttfunctions.cpp tttfunctions.hpp
	$(CC) $(FLAGS) -c $<

mynetcat.o: mynetcat.cpp
	$(CC) $(FLAGS) -c $<

mync_client.o: mync_client.cpp mync_client.hpp
	$(CC) $(FLAGS) -c $<

mync_server.o: mync_server.cpp mync_server.hpp
	$(CC) $(FLAGS) -c $<

clean: 
	rm -f *.o mync ttt