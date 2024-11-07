#ifndef MEMTABLE_H
#define MEMTABLE_H

#include "avl_tree.h"
#include <string>
#include <vector>

class Memtable {
  private:
    AVLTree t;
    size_t max_size; // The maximum size of the memtable
    size_t current_size; // The current size of the memtable

  public:
    Memtable(size_t memtable_size);

    void put(int key, int value);
    int get(int key);
    std::vector<std::pair<int, int>> scan(int key1, int key2);
    
    size_t getSize() const;
    bool isFull() const;
    void clear();
}

#endif