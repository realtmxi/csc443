#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <string>
#include <vector>

#include "avl_tree.h"

class Memtable
{
   private:
    AVLTree t;
    // The maximum amount of key-value pairs that can be stored in the Memtable.
    size_t max_size;
    size_t current_size;

   public:
    Memtable();
    explicit Memtable(size_t memtable_size);

    void put(int key, int value);
    int get(int key);
    std::vector<std::pair<int, int>> scan(int key1, int key2);

    size_t getSize() const;
    bool isFull() const;
    void clear();
};

#endif