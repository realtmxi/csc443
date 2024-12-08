#include "database.h"

#include <fcntl.h>   // for open
#include <unistd.h>  // for pread

#include <algorithm>
#include <chrono>  // for using timestamps
#include <climits>
#include <filesystem>  // for using filesystem to check if directory exists
#include <fstream>     // for reading and writing files
#include <set>         // for using set to track found keys
#include <sstream>     // for using stringstream to create filenames

#include "b_tree/b_tree.h"
#include "b_tree/b_tree_manager.h"
#include "bloom_filter/bloom_filter.h"
#include "config.h"

Database::Database(const std::string& name, size_t memtableSize,
                   bool use_binary_search)
    : db_name_(name),
      memtable_(memtableSize),
      use_binary_search_(use_binary_search),
      is_open_(false),
      buffer_pool_(MAX_BUFFER_POOL_SIZE)
{
    // Ensure the database name doesn't end with a slash
    if (db_name_.back() == '/')
    {
        db_name_.pop_back();
    }
}

void
Database::Open()
{
    if (!std::filesystem::exists(db_name_))
    {
        std::filesystem::create_directory(db_name_);
    }
    else
    {
        for (const auto& entry : std::filesystem::directory_iterator(db_name_))
        {
            // Only include `.sst` files in the list
            if (entry.path().extension() == ".sst")
            {
                sst_files_.push_back(entry.path().string());
            }

            // Load the Bloom filters for each SST file
            if (entry.path().extension() == ".filter")
            {
                BloomFilter bloom_filter(entry.path().string());
                // remove the .filter extension and add to the map
                std::string sst_file = entry.path().string();
                sst_file = sst_file.substr(0, sst_file.size() - 7);
                bloom_filters_.insert({sst_file, bloom_filter});
            }
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
        return;
    }

    memtable_.Put(key, value);
    if (memtable_.IsFull())
    {
        StoreMemtable();
    }
}

void
Database::Delete(int key)
{
    if (!is_open_)
    {
        return;
    }

    memtable_.Delete(key);
}

int
Database::Get(int key)
{
    if (!is_open_)
    {
        return -1;
    }

    // Check memtable first
    auto result = memtable_.Get(key);
    if (result == INT_MAX)
    {
        return -1;
    }
    if (result != -1)
    {
        return result;
    }

    // Loop through SST files in reverse order
    for (auto it = sst_files_.rbegin(); it != sst_files_.rend(); ++it)
    {
        // Load the Bloom filter for the current SST file
        auto bloom_filter = bloom_filters_.find(*it)->second;

        // Check Bloom filter
        if (!bloom_filter.MayContain(key))
        {
            continue;
        }

        // Search the SST file using the BTreeManager
        BTreeManager btm(*it, GetLargestLSMLevel(), buffer_pool_);
        if (use_binary_search_)
        {
            result = btm.BinarySearchGet(key);
        }
        else
        {
            result = btm.Get(key);
        }

        if (result == INT_MAX)
        {
            return -1;
        }
        if (result != -1 && result != INT_MAX)
        {
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
        return {};
    }

    std::vector<std::pair<int, int>> results;
    int range = key2 - key1;

    // Scan memory table first
    auto memtable_results = memtable_.Scan(key1, key2);
    results.insert(results.end(), memtable_results.begin(),
                   memtable_results.end());

    // Use set to track found keys
    std::set<int> result_keys;
    for (const auto& r : results)
    {
        result_keys.insert(r.first);
    }

    // Return if we have all possible keys
    if (result_keys.size() > static_cast<size_t>(range))
    {
        return results;
    }

    // Go through SST files in reverse order
    for (auto it = sst_files_.rbegin(); it != sst_files_.rend(); ++it)
    {
        // Scan the SST file using the BTreeManager
        BTreeManager btm(*it, GetLargestLSMLevel(), buffer_pool_);
        auto sst_results = btm.Scan(key1, key2);

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

void
Database::StoreMemtable()
{
    // Generate a unique filename for the SST file
    std::string filename = GenerateFileName();

    // Get all kv pairs from the memtable in sorted order
    auto result = memtable_.Scan(INT_MIN, INT_MAX);

    // Create a BloomFilter and populate it with keys from the memtable. 8
    // bits
    // per entry
    BloomFilter bloom_filter(BLOOM_FILTER_BITS);

    for (const auto& pair : result)
    {
        bloom_filter.Insert(pair.first);
    }

    // Serialize the BloomFilter to disk alongside the SST file
    bloom_filter.SerializeToDisk(filename + ".filter");

    // add the bloom filter to the map
    bloom_filters_.insert({filename, bloom_filter});
    // Use BTree to store the data
    BTree btree(result);
    btree.SaveBTreeToDisk(filename);

    // Add the SST file to the list of SST files
    sst_files_.push_back(filename);

    // Clear the memtable
    memtable_.Clear();

    // Compact if necessary
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

    BTreeManager btm(filename1, GetLargestLSMLevel(), buffer_pool_);
    std::string out_file = btm.Merge(filename2);
    std::filesystem::rename(out_file, db_name_ + "/" + out_file);

    // Load Bloom filters for both files
    BloomFilter bloom_filter1(filename1 + ".filter");
    BloomFilter bloom_filter2(filename2 + ".filter");

    // Create a new Bloom filter as the union of both
    BloomFilter merged_filter(BLOOM_FILTER_BITS);
    merged_filter.Union(bloom_filter1);
    merged_filter.Union(bloom_filter2);

    // Serialize the merged Bloom filter to disk
    std::string out_filter = db_name_ + "/" + out_file + ".filter";
    merged_filter.SerializeToDisk(out_filter);

    // remove the merged files
    std::filesystem::remove(filename1);
    std::filesystem::remove(filename1 + ".filter");
    std::filesystem::remove(filename2);
    std::filesystem::remove(filename2 + ".filter");
    sst_files_.pop_back();
    sst_files_.pop_back();

    // add the new merged file to the list of SST files
    sst_files_.push_back(db_name_ + "/" + out_file);

    // add the new bloom filter to the map
    bloom_filters_.insert({db_name_ + "/" + out_file, merged_filter});
    bloom_filters_.erase(filename1 + ".filter");
    bloom_filters_.erase(filename2 + ".filter");

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