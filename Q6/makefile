.PHONY: all clean

CC = g++
FLAGS = -Wall -Wextra -g -fprofile-arcs -ftest-coverage

all: mync ttt

mync: mynetcat.o TCPclient.o TCPserver.o UDPclient.o UDPserver.o UDSserver.o UDSclient.o
	$(CC) $(FLAGS) -o $@ $^

ttt: ttt.o tttfunctions.o
	$(CC) $(FLAGS) -o $@ $^

ttt.o: ttt.cpp
	$(CC) $(FLAGS) -c $<

tttfunctions.o: tttfunctions.cpp tttfunctions.hpp
	$(CC) $(FLAGS) -c $<

mynetcat.o: mynetcat.cpp
	$(CC) $(FLAGS) -c $<

TCPclient.o: TCPclient.cpp TCPclient.hpp
	$(CC) $(FLAGS) -c $<

TCPserver.o: TCPserver.cpp TCPserver.hpp
	$(CC) $(FLAGS) -c $<

UDPclient.o: UDPclient.cpp UDPclient.hpp
	$(CC) $(FLAGS) -c $<

UDPserver.o: UDPserver.cpp UDPserver.hpp
	$(CC) $(FLAGS) -c $<

UDSserver.o: UDSserver.cpp UDSserver.hpp
	$(CC) $(FLAGS) -c $<

UDSclient.o: UDSclient.cpp UDSclient.hpp
	$(CC) $(FLAGS) -c $<

clean: 
	rm -f *.o mync ttt *.gcda *.gcno *.gcov

gcov_report: mync ttt
	./run_tests.sh

	gcov ttt.cpp tttfunctions.cpp mynetcat.cpp TCPclient.cpp TCPserver.cpp UDPserver.cpp UDPclient.cpp UDSserver.cpp UDSclient.cpp

