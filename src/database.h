#ifndef DATABASE_H
#define DATABASE_H

#include <string>

#include "avl_tree.h"

class Database
{
   private:
    std::string dbName;
    AVLTree memtable;
    size_t memtableSize;
    bool isOpen;
    std::vector<std::string> sstFiles;
    void StoreMemtable();
    AVLTree LoadMemtable(const std::string& filename);

   public:
    Database(const std::string& name, size_t memtableSize = 128);
    void Open();
    void Close();
    void Put(int key, int value);
    int Get(int key);
    std::vector<std::pair<int, int>> Scan(int key1, int key2);
};

#endif
