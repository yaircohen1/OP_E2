CC = g++
CFLAGS = -Wall -Wextra -std=c++11

all: ttt

# Rule to link the object files into the final executable
ttt: ttt.o tttfunctions.o
	$(CC) $(CFLAGS) -o ttt ttt.o tttfunctions.o

# Rule to compile the source files into object files
%.o: %.cpp tttfunctions.hpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build files
clean:
	rm -f ttt *.o

# Phony targets
.PHONY: all clean