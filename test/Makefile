CC = g++
CXXFLAGS += -std=c++0x -Wall -O3

all: internal various

internal: test.cpp 
	$(CC) $(CXXFLAGS) test.cpp -o test.out

various: test_various.cpp 
	$(CC) $(CXXFLAGS) test_various.cpp -o various.out

clean:
	rm test.out various.out
