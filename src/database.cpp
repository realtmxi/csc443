#include "database.h"

#include <fcntl.h>   // for open
#include <unistd.h>  // for pread

#include <chrono>  // for using timestamps
#include <climits>
#include <filesystem>  // for using filesystem to check if directory exists
#include <fstream>     // for reading and writing files
#include <set>         // for using set to track found keys
#include <sstream>     // for using stringstream to create filenames

#include "b_tree/b_tree.h"
#include "b_tree/b_tree_manager.h"

Database::Database(const std::string& name, size_t memtableSize)
    : dbName(name),
      memtable(memtableSize),
      memtableSize(memtableSize),
      isOpen(false)
{
}

void
Database::Open()
{
    if (!std::filesystem::exists(dbName))
    {
        std::filesystem::create_directory(dbName);
        printf("Created Database: %s\n", dbName.c_str());
    }
    else
    {
        printf("Database already exists: %s\nLoading files...\n",
               dbName.c_str());
        for (const auto& entry : std::filesystem::directory_iterator(dbName))
        {
            sstFiles.push_back(entry.path().string());
        }
    }
    isOpen = true;
}

void
Database::Close()
{
    printf("Closing Database: %s\n", dbName.c_str());
    if (memtable.getSize() > 0)
    {
        StoreMemtable();
    }
    isOpen = false;
}

void
Database::Put(int key, int value)
{
    if (!isOpen)
    {
        printf("Database is not open\n");
        return;
    }

    memtable.put(key, value);
    if (memtable.getSize() >= memtableSize)
    {
        printf("Memtable is full\n");
        StoreMemtable();
    }
}

int
Database::Get(int key)
{
    if (!isOpen)
    {
        printf("Database is not open\n");
        return -1;
    }

    // check memtable first
    auto result = memtable.get(key);
    if (result != -1)
    {
        return result;
    }

    // go through SST files
    for (const auto& file : sstFiles)
    {
        BTreeManager btm(file);
        result = btm.Get(key);
        if (result != -1)
        {
            return result;
        }
    }

    return -1;
}

std::vector<std::pair<int, int>>
Database::Scan(int key1, int key2)
{
    if (!isOpen)
    {
        printf("Database is not open\n");
        return {};
    }

    std::vector<std::pair<int, int>> results;
    int range = key2 - key1;

    // scan memory table first
    auto memtableResults = memtable.scan(key1, key2);
    results.insert(results.end(), memtableResults.begin(),
                   memtableResults.end());

    // use set to track found keys
    std::set<int> resultKeys;
    for (const auto& r : results)
    {
        resultKeys.insert(r.first);
    }

    // return if we have all possible keys
    if (resultKeys.size() > static_cast<size_t>(range))
    {
        return results;
    }

    // Go through SST files then
    for (const auto& file : sstFiles)
    {
        printf("Scanning SST file: %s\n", file.c_str());
        BTreeManager btm(file);
        auto sst_results = btm.Scan(key1, key2);
        // add to results if not already found
        for (const auto& r : sst_results)
        {
            if (resultKeys.find(r.first) == resultKeys.end())
            {
                results.push_back(r);
                resultKeys.insert(r.first);
                if (resultKeys.size() > static_cast<size_t>(range))
                {
                    return results;
                }
            }
        }
    }

    return results;
}

/* Transform the memtable into an SST when it reaches capacity. */
void
Database::StoreMemtable()
{
    // Generate a unique filename for the SST file.
    std::string filename = _generateFileName();

    // Get all kv pairs from the memtable in sorted order.
    auto result = memtable.scan(INT_MIN, INT_MAX);

    // Use BTree to store the data
    BTree btree(result);
    btree.SaveBTreeToDisk(filename);

    // Add the SST file to the list of SST files.
    sstFiles.push_back(filename);

    memtable.clear();

    printf("Stored Memtable to SST file: %s\n", filename.c_str());
}

/* A helper function for StoreMemtable. Generate a unique filename for each
   SST file using the current timestamp. */
std::string
Database::_generateFileName()
{
    // get current time in microseconds
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                      now.time_since_epoch())
                      .count();

    // Save the filename as sst_level_timestamp.sst. It is always 0 on first
    // save.
    std::stringstream filename;
    filename << dbName << "/sst_0_" << (now_ms) << ".sst";
    return filename.str();
}
