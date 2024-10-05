CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: main tests

main: main.cpp kv_store.cpp avl_tree.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp kv_store.cpp avl_tree.cpp

tests: tests.cpp kv_store.cpp avl_tree.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp kv_store.cpp avl_tree.cpp

clean:
	rm -f main tests
