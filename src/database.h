#ifndef DATABASE_H
#define DATABASE_H

#include <string>

#include "memtable.h"
#include "sst.h"

class Database
{
   private:
    std::string db_name_;
    Memtable memtable_;
    bool is_open_;
    std::vector<std::string> sst_files_;
    void StoreMemtable();
    std::string GenerateFileName();
    void Compact();
    int GetLargestLSMLevel();

   public:
    Database(const std::string& name, size_t memtableSize);
    void Open();
    void Close();
    void Put(int key, int value);
    int Get(int key);
    void Delete(int key);
    std::vector<std::pair<int, int>> Scan(int key1, int key2);
};

#endif
