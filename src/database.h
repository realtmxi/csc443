#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <unordered_map>

#include "bloom_filter/bloom_filter.h"
#include "buffer_pool/buffer_pool.h"
#include "memtable.h"
#include "sst.h"

class Database
{
   private:
    std::string db_name_;
    Memtable memtable_;
    bool use_binary_search_;
    bool is_open_;
    BufferPool buffer_pool_;
    std::vector<std::string> sst_files_;
    std::unordered_map<std::string, BloomFilter> bloom_filters_;
    void StoreMemtable();
    std::string GenerateFileName();
    void Compact();
    int GetLargestLSMLevel();

   public:
    Database(const std::string& name, size_t memtableSize,
             bool use_binary_search = false);
    void Open();
    void Close();
    void Put(int key, int value);
    int Get(int key);
    void Delete(int key);
    std::vector<std::pair<int, int>> Scan(int key1, int key2);
};

#endif
