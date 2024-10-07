CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

CFILES = src/main.cpp src/avl_tree.cpp src/database.cpp test/tests.cpp
HFILES = src/avl_tree.h src/database.h

main: src/main.cpp src/avl_tree.cpp src/database.cpp
	$(CXX) $(CXXFLAGS) -o main src/main.cpp src/avl_tree.cpp src/database.cpp

tests: test/tests.cpp src/avl_tree.cpp src/database.cpp
	$(CXX) $(CXXFLAGS) -o tests test/tests.cpp src/avl_tree.cpp src/database.cpp

clean:
	rm -f main tests

test: tests
	./tests

format:
	clang-format -i $(CFILES) $(HFILES)