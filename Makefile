# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++17 -Wall -Wextra -O2

# Source and header files
CFILES = src/main.cpp \
         src/avl_tree.cpp \
         src/database.cpp \
         src/memtable.cpp \
         src/b_tree/b_tree.cpp \
         src/b_tree/b_tree_page.cpp \
		 src/b_tree/b_tree_manager.cpp \
         src/sst.cpp \
        #  test/tests.cpp

HFILES = src/avl_tree.h \
         src/database.h \
         src/memtable.h \
         src/b_tree/b_tree.h \
         src/b_tree/b_tree_page.h \
		 src/b_tree/b_tree_manager.h \
		 src/include/common/config.h \
         src/sst.h \



main: $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -o main $(CFILES)

tests: $(CFILES)
	$(CC) $(CFLAGS) -o tests test/tests.cpp src/avl_tree.cpp src/database.cpp src/memtable.cpp src/sst.cpp

clean:
	rm -f main tests

test: tests
	./tests

format:
	clang-format -i $(CFILES) $(HFILES)