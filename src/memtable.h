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
    int max_size_;

   public:
    Memtable();
    explicit Memtable(int memtable_size);

    void Put(int key, int value);
    int Get(int key);
    std::vector<std::pair<int, int>> Scan(int key1, int key2);

    int GetSize();
    bool IsFull();
    void Clear();
    void Delete(int key);
};

#endif