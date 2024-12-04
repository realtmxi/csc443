#include "database.h"

#include <fcntl.h>   // for open
#include <unistd.h>  // for pread

#include <chrono>  // for using timestamps
#include <climits>
#include <filesystem>  // for using filesystem to check if directory exists
#include <fstream>     // for reading and writing files
#include <set>         // for using set to track found keys
#include <sstream>     // for using stringstream to create filenames
#include <algorithm>

#include "b_tree/b_tree.h"
#include "b_tree/b_tree_manager.h"

Database::Database(const std::string& name, size_t memtableSize)
    : db_name_(name), memtable_(memtableSize), is_open_(false)
{
}

void
Database::Open()
{
    if (!std::filesystem::exists(db_name_))
    {
        std::filesystem::create_directory(db_name_);
        printf("Created Database: %s\n", db_name_.c_str());
    }
    else
    {
        printf("Database already exists: %s\nLoading files...\n",
               db_name_.c_str());
        for (const auto& entry : std::filesystem::directory_iterator(db_name_))
        {
            sst_files_.push_back(entry.path().string());
        }
    }
    // Sort descending order to maintain LSM levels
    std::sort(sst_files_.begin(), sst_files_.end(),
              std::greater<std::string>());
    is_open_ = true;
}

void
Database::Close()
{
    printf("Closing Database: %s\n", db_name_.c_str());
    if (memtable_.GetSize() > 0)
    {
        StoreMemtable();
    }
    is_open_ = false;
}

void
Database::Put(int key, int value)
{
    if (!is_open_)
    {
        printf("Database is not open\n");
        return;
    }

    memtable_.Put(key, value);
    if (memtable_.IsFull())
    {
        printf("Memtable is full\n");
        StoreMemtable();
    }
}

void
Database::Delete(int key)
{
    if (!is_open_)
    {
        printf("Database is not open\n");
        return;
    }

    memtable_.Delete(key);
}

int
Database::Get(int key)
{
    if (!is_open_)
    {
        printf("Database is not open\n");
        return -1;
    }

    // check memtable first
    auto result = memtable_.Get(key);
    if (result == INT_MAX)
    {
        printf("Key %d is a tombstone in memtable\n", key);
        return -1;
    }
    if (result != -1)
    {
        printf("Found key %d in memtable\n", key);
        return result;
    }

    // loop through sstfiles in reverse order
    for (auto it = sst_files_.rbegin(); it != sst_files_.rend(); ++it)
    {
        BTreeManager btm(*it, GetLargestLSMLevel());
        result = btm.Get(key);
        if (result == INT_MAX)
        {
            printf("Key %d is a tombstone in sstfile\n", key);
            return -1;
        }
        if (result != -1 && result != INT_MAX)
        {
            // print the filename it was found in
            printf("Found key %d in %s\n", key, it->c_str());
            return result;
        }
    }

    return -1;
}

std::vector<std::pair<int, int>>
Database::Scan(int key1, int key2)
{
    if (!is_open_)
    {
        printf("Database is not open\n");
        return {};
    }

    std::vector<std::pair<int, int>> results;
    int range = key2 - key1;

    // scan memory table first
    auto memtable_results = memtable_.Scan(key1, key2);
    results.insert(results.end(), memtable_results.begin(),
                   memtable_results.end());
    // if results not empty, print where it was found
    if (!memtable_results.empty())
    {
        // print the min and max keys found in the memtable
        printf("Found keys in memtable: %d to %d\n", memtable_results[0].first,
               memtable_results[memtable_results.size() - 1].first);
    }

    // use set to track found keys
    std::set<int> result_keys;
    for (const auto& r : results)
    {
        result_keys.insert(r.first);
    }

    // return if we have all possible keys
    if (result_keys.size() > static_cast<size_t>(range))
    {
        return results;
    }

    // Go through SST files in reverse order
    for (auto it = sst_files_.rbegin(); it != sst_files_.rend(); ++it)
    {
        printf("Scanning SST file: %s\n", it->c_str());
        BTreeManager btm(*it, GetLargestLSMLevel());
        auto sst_results = btm.Scan(key1, key2);

        if (!sst_results.empty())
        {
            printf("Found keys in %s\n", it->c_str());
            printf("Keys: %d to %d\n", sst_results[0].first,
                   sst_results[sst_results.size() - 1].first);
        }
        // add to results if not already found
        for (const auto& r : sst_results)
        {
            if (result_keys.find(r.first) == result_keys.end())
            {
                results.push_back(r);
                result_keys.insert(r.first);
                if (result_keys.size() > static_cast<size_t>(range))
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
    std::string filename = GenerateFileName();
    printf("Storing memtable to %s\n", filename.c_str());

    // Get all kv pairs from the memtable in sorted order.
    auto result = memtable_.Scan(INT_MIN, INT_MAX);
    printf("Memtable size: %lu\n", result.size());

    // Use BTree to store the data
    BTree btree(result);
    btree.SaveBTreeToDisk(filename);

    // Add the SST file to the list of SST files.
    sst_files_.push_back(filename);

    memtable_.Clear();

    Compact();
}

/* A helper function for StoreMemtable. Generate a unique filename for each
   SST file using the current timestamp. */
std::string
Database::GenerateFileName()
{
    // get current time in microseconds
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                      now.time_since_epoch())
                      .count();

    // Save the filename as sst_level_timestamp.sst. It is always 0000 on first
    // save.
    std::stringstream filename;
    filename << db_name_ << "/sst_0000_" << (now_ms) << ".sst";
    return filename.str();
}

void
Database::Compact()
{
    // To compact, we will look at the most recent 2 SST files. If they have
    // the same level, we will use BTreeManager to merge them into a new SST.
    // If this new SST shares a level with the next most recent SST, we will
    // merge them as well. We will continue this process until we reach an SST
    // with a different level.

    // sst_files_ are sorted by timestamp, so the most recent SST is at the end.
    if (sst_files_.size() < 2)
    {
        return;
    }

    std::string filename1 = sst_files_[sst_files_.size() - 1];
    std::string filename2 = sst_files_[sst_files_.size() - 2];

    // check if they each have the same level. if not, return an error
    // search for "sst_" and get the next 4 characters. The filename includes
    // the path
    std::string level1 = filename1.substr(filename1.find("sst_") + 4, 4);
    std::string level2 = filename2.substr(filename2.find("sst_") + 4, 4);
    if (level1 != level2)
    {
        return;
    }
    printf("Merging %s and %s\n", filename1.c_str(), filename2.c_str());

    BTreeManager btm(filename1, GetLargestLSMLevel());
    std::string out_file = btm.Merge(filename2);
    std::filesystem::rename(out_file, db_name_ + "/" + out_file);

    // remove the merged files
    std::filesystem::remove(filename1);
    std::filesystem::remove(filename2);
    sst_files_.pop_back();
    sst_files_.pop_back();

    // add the new merged file to the list of SST files
    sst_files_.push_back(db_name_ + "/" + out_file);

    // recursively compact
    Compact();
}

int
Database::GetLargestLSMLevel()
{
    if (sst_files_.empty())
    {
        return 0;
    }

    // search for "sst_" and get the next 4 characters
    std::string level = sst_files_[0].substr(sst_files_[0].find("sst_") + 4, 4);
    return std::stoi(level);
}