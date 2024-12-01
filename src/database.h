#ifndef DATABASE_H
#define DATABASE_H

#include <string>

#include "memtable.h"
#include "sst.h"

class Database
{
   private:
    std::string dbName;
    Memtable memtable;
    size_t memtableSize;
    bool isOpen;
    std::vector<std::string> sstFiles;
    void StoreMemtable();
    std::string _generateFileName();

   public:
    Database(const std::string& name, size_t memtableSize);
    void Open();
    void Close();
    void Put(int key, int value);
    int Get(int key);
    std::vector<std::pair<int, int>> Scan(int key1, int key2);
};

#endif
