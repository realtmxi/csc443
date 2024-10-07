#include "database.h"

#include <fcntl.h>   // for open
#include <unistd.h>  // for pread

#include <ctime>       // for using time to create unique filenames
#include <filesystem>  // for using filesystem to check if directory exists
#include <fstream>     // for reading and writing files
#include <set>         // for using set to track found keys
#include <sstream>     // for using stringstream to create filenames

Database::Database(const std::string& name, size_t memtableSize)
    : dbName(name), memtableSize(memtableSize), isOpen(false) {}

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
        printf("Database already exists: %s\nLoading files...\n", dbName.c_str());
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

    auto result = memtable.get(key);
    if (result != -1)
    {
        return result;
    }

    // go through SST files
    // TODO: This is just a placeholder for an easy get.
    // we need to do binary search over the sst files in pages instead of just loading it all.
    for (const auto& file : sstFiles)
    {
        auto tree = LoadMemtable(file);
        result = tree.get(key);
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
    results.insert(results.end(), memtableResults.begin(), memtableResults.end());

    // use set to track found keys
    std::set<int> resultKeys;
    for (const auto& r : results)
    {
        resultKeys.insert(r.first);
    }

    // go through SST files second
    for (const auto& file : sstFiles)
    {
        auto tree = LoadMemtable(file);
        auto fileResults = tree.scan(key1, key2);
        // add to results if not already there
        for (const auto& r : fileResults)
        {
            if (resultKeys.find(r.first) == resultKeys.end())
            {
                results.push_back(r);
                resultKeys.insert(r.first);
            }
        }
    }

    return results;
}

void
Database::StoreMemtable()
{
    // make a unique filename using current time
    std::time_t t = std::time(nullptr);
    std::stringstream filename;
    filename << dbName << "/sst_" << t << ".sst";

    // opening file
    std::ofstream outFile(filename.str(), std::ios::binary);
    if (!outFile.is_open())
    {
        printf("Failed to create SST file: %s\n", filename.str().c_str());
        return;
    }

    // writing to disk and wiping memtable
    auto result = memtable.scan(INT_MIN, INT_MAX);
    for (auto& r : result)
    {
        outFile.write(reinterpret_cast<const char*>(&r.first), sizeof(r.first));
        outFile.write(reinterpret_cast<const char*>(&r.second), sizeof(r.second));
    }

    outFile.close();
    sstFiles.push_back(filename.str());
    memtable.clear();
}

AVLTree
Database::LoadMemtable(const std::string& filename)
{
    AVLTree tree;

    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0)
    {
        printf("Failed to open SST file: %s\n", filename.c_str());
        return tree;
    }

    // Read the file into the tree.
    // TODO: This is just a placeholder for an easy get/scan.
    // We need to implement a binary search over the ssts for get and return the latest value for a key.
    // The assignment says to use pages, but we have not used pages so far. Perhaps it is just to load a chunk of the file at a time in sorted order?
    // For scan, this might be good enough for now.
    int key, value;
    while (read(fd, &key, sizeof(key)) > 0 && read(fd, &value, sizeof(value)) > 0)
    {
        tree.put(key, value);
    }

    close(fd);
    return tree;
}