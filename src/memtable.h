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
    size_t max_size_;
    size_t current_size_;

   public:
    Memtable();
    explicit Memtable(size_t memtable_size);

    void Put(int key, int value);
    int Get(int key);
    std::vector<std::pair<int, int>> Scan(int key1, int key2);

    size_t GetSize() const;
    bool IsFull() const;
    void Clear();
};

#endif