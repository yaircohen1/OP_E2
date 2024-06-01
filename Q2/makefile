.PHONY: all clean

CC = g++
FLAGS = -g -std=c++11

all: mync ttt

mync: mync.o
	$(CC) $(FLAGS) -o $@ $^

ttt: ttt.o tttfunctions.o
	$(CC) $(FLAGS) -o $@ $^

mync.o: mync.cpp
	$(CC) $(FLAGS) -c $<

ttt.o: ttt.cpp
	$(CC) $(FLAGS) -c $<

tttfunctions.o: tttfunctions.cpp tttfunctions.hpp
	$(CC) $(FLAGS) -c $<

clean: 
	rm -f *.o mync ttt