CC = g++
CXXFLAGS += -g -std=c++0x -O3 -Wall -pedantic -Wno-parentheses -DHAS_COLOR

all: various.out

various.out: test_various.cpp
	$(CC) $(CXXFLAGS) test_various.cpp -o various.out

clean:
	rm various.out
