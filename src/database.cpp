#include "database.h"

#include <fcntl.h>   // for open
#include <unistd.h>  // for pread

#include <climits>
#include <ctime>       // for using time to create unique filenames
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

    // Go through SST files then
    for (const auto& file : sstFiles)
    {
        BTreeManager btm(file);
        auto sst_results = btm.Scan(key1, key2);
        // add to results if not already found
        for (const auto& r : sst_results)
        {
            if (resultKeys.find(r.first) == resultKeys.end())
            {
                results.push_back(r);
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
}

/* A helper function for StoreMemtable. Generate a unique filename for each
   SST file using the current timestamp. */
std::string
Database::_generateFileName()
{
    // Get the current time for filename uniqueness.
    std::time_t t = std::time(nullptr);

    // Creat a filename with the current timestamp and level (first save is 0).
    std::stringstream filename;
    filename << dbName << "/sst_0_" << t << ".sst";

    return filename.str();
}