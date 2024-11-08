# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++17 -Wall -Wextra -O2

# Source and header files
CFILES = src/main.cpp src/avl_tree.cpp src/database.cpp src/memtable.cpp src/sst.cpp test/tests.cpp
HFILES = src/avl_tree.h src/database.h src//memtable.h src/sst.h

main: src/main.cpp src/avl_tree.cpp src/database.cpp src/memtable.cpp src/sst.cpp
	$(CC) $(CFLAGS) -o main src/main.cpp src/avl_tree.cpp src/database.cpp src/memtable.cpp src/sst.cpp

tests: test/tests.cpp src/avl_tree.cpp src/database.cpp src/memtable.cpp src/sst.cpp
	$(CC) $(CFLAGS) -o tests test/tests.cpp src/avl_tree.cpp src/database.cpp src/memtable.cpp src/sst.cpp

clean:
	rm -f main tests

test: tests
	./tests

format:
	clang-format -i $(CFILES) $(HFILES)