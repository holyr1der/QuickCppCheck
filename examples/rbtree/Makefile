#CXX = /home/zeus/build/bin/clang++
CXX = g++
CXXFLAGS += -g -std=c++0x -O3 -W -Wall -pedantic -Wno-parentheses -Wno-unused-parameter -DHAS_COLOR

all: rbtree_quicktest

rbtree_quicktest: rbtree_quicktest.cc rbtree.hpp rbtree.cc
	$(CXX) $(CXXFLAGS) rbtree_quicktest.cc -o rbtree_quicktest

clean:
	rm rbtree_quicktest
