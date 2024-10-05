CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SOURCES = main.cpp kv_store.cpp avl_tree.cpp tests.cpp
HEADERS = avl_tree.h kv_store.h

all: main tests

main: main.cpp kv_store.cpp avl_tree.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp kv_store.cpp avl_tree.cpp

tests: tests.cpp kv_store.cpp avl_tree.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp kv_store.cpp avl_tree.cpp

format:
	clang-format -i $(SOURCES) $(HEADERS)

clean:
	rm -f main tests
