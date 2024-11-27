#include "database.h"

#include <fcntl.h>   // for open
#include <unistd.h>  // for pread

#include <climits>
#include <ctime>       // for using time to create unique filenames
#include <filesystem>  // for using filesystem to check if directory exists
#include <fstream>     // for reading and writing files
#include <set>         // for using set to track found keys
#include <sstream>     // for using stringstream to create filenames

#include "lsm_tree.h"  // for merging BTree files

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

    auto result = memtable.get(key);
    if (result != -1)
    {
        return result;
    }

    // go through SST files
    for (const auto& file : sstFiles)
    {
        SSTable sst(file);
        result = sst.get(key);

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
        SSTable sst(file);
        auto sstResults = sst.scan(key1, key2);
        for (const auto& r : sstResults)
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

/* Transform the memtable into an SST when it reaches capacity. */
void
Database::StoreMemtable()
{
    // Generate a unique filename for the SST file.
    std::string filename = _generateFileName();

    // Get all kv pairs from the memtable in sorted order.
    auto result = memtable.scan(INT_MIN, INT_MAX);

    // Use SSTable to write the data into disk.
    SSTable sst(filename);
    sst.write(filename, result);

    // Add the SST file to the list of SST files.
    sstFiles.push_back(filename);

    memtable.clear();

    // Compact the SST files if necessary.
    _compact();
}

void
Database::_compact()
{
    // SST files are sorted by their timestamp, oldest to newest, in the form of
    // "sst_{level}_{timestamp}.sst" This also implies that the levels are too
    // sorted in descending order, since no compaction for a level can occur
    // until the previous level is compacted. Therefore, the last two files in
    // the list will be the only two files that can share the same level.

    // If there are less than 2 files, no compaction is necessary.
    if (sstFiles.size() < 2)
    {
        return;
    }

    // Check the last 2 files, if they are of the same level, merge them. Repeat
    // until the last 2 files are of different levels.
    auto lastFile = sstFiles.back();
    auto secondLastFile = sstFiles[sstFiles.size() - 2];

    // Get the level of the last file.
    std::string delimiter = "_";
    auto level = lastFile.substr(4, lastFile.find(delimiter, 4) - 4);

    // If the second to last file has the same level, merge the two files.
    if (secondLastFile.substr(4, secondLastFile.find(delimiter, 4) - 4) ==
        level)
    {
        // Merge the two files.
        LSMTree lsm;
        lsm.MergeBtrees(lastFile, secondLastFile);

        // Remove the two files from the list of SST files.
        sstFiles.pop_back();
        sstFiles.pop_back();

        // Add the new file to the list of SST files.
        sstFiles.push_back(lsm.GetFilename());

        // Continue compacting if necessary.
        _compact();
    }
}

/* A helper function for StoreMemtable. Generate a unique filename for each
   SST file using the current timestamp. */
std::string
Database::_generateFileName()
{
    // Get the current time for filename uniqueness.
    std::time_t t = std::time(nullptr);

    // Creat a filename with the level (0) and the current timestamp.
    std::stringstream filename;
    filename << dbName << "/sst_0_" << t << ".sst";

    return filename.str();
}