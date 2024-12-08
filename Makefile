# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++17 -Wall -Wextra -O2

SHARED_C_FILES = src/avl_tree.cpp \
             src/database.cpp \
             src/memtable.cpp \
             src/sst.cpp \
             src/b_tree/b_tree.cpp \
             src/b_tree/b_tree_page.cpp \
             src/b_tree/b_tree_manager.cpp \
             src/bloom_filter/bloom_filter.cpp \
             src/buffer_pool/buffer_pool.cpp

SHARED_H_FILES = src/avl_tree.h \
         src/database.h \
         src/memtable.h \
         src/b_tree/b_tree.h \
         src/b_tree/b_tree_page.h \
         src/b_tree/b_tree_manager.h \
         src/include/common/config.h \
         src/sst.h \
         src/bloom_filter/bloom_filter.h \
         src/buffer_pool/buffer_pool.h

main: $(SHARED_C_FILES) $(SHARED_H_FILES) src/main.cpp
	$(CC) $(CFLAGS) -o main src/main.cpp $(SHARED_C_FILES)

tests: $(SHARED_C_FILES) $(SHARED_H_FILES) test/tests.cpp
	$(CC) $(CFLAGS) -o tests test/tests.cpp $(SHARED_C_FILES)

clean:
	rm -f main tests
	rm -f ../*.sst ../*.filter

test: tests
	./tests

format:
	clang-format -i src/main.cpp test/tests.cpp $(SHARED_C_FILES) $(SHARED_H_FILES)