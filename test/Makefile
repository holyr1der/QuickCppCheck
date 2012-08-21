CC = g++
CXXFLAGS += -std=c++0x -O3 -Wall -Wno-parentheses

all: internal various

internal: test.cpp 
	$(CC) $(CXXFLAGS) test.cpp -o internal.out

various: test_various.cpp 
	$(CC) $(CXXFLAGS) test_various.cpp -o various.out

clean:
	rm internal.out various.out
