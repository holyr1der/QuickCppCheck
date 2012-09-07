#CC = /home/zeus/build/bin/clang++
CC = g++
CXXFLAGS += -g -std=c++0x -O3 -Wall -pedantic -Wno-parentheses -DHAS_COLOR

all: internal.out

internal.out: test.cpp
	$(CC) $(CXXFLAGS) test.cpp -o internal.out

clean:
	rm internal.out
