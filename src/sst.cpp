#include "sst.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <stdexcept>

SSTable::SSTable(const std::string& path)
    : file_path(path), bloom_filter(1024, 3)
{
    struct stat buf;
    if (stat(file_path.c_str(), &buf) != 0)
    {
        throw std::runtime_error("Failed to stat SSTable file");
    }
    else
    {
        // Dividing the file size by the size of one kv pair.
        num_entries = buf.st_size / (sizeof(int) * 2);

        // Deserialize the Bloom filter associated with this SSTable
        std::string bloom_filter_file = file_path + ".filter";
        bloom_filter.DeserializeFromDisk(bloom_filter_file);
    }
}

int
SSTable::readKey(int fd, off_t offset)
{
    int key;
    if (pread(fd, &key, sizeof(int), offset) != sizeof(int))
    {
        throw std::runtime_error("Failed to read key from SSTable");
    }
    return key;
}

int
SSTable::readValue(int fd, off_t offset)
{
    int value;
    if (pread(fd, &value, sizeof(int), offset) != sizeof(int))
    {
        throw std::runtime_error("Failed to read value from SSTable");
    }
    return value;
}
/* Use a binary search to find the start of the range and continue with a scan
  until encounter a key key outside of the range. */
std::vector<std::pair<int, int>>
SSTable::scan(int key1, int key2)
{
    std::vector<std::pair<int, int>> result;

    // Check if any keys in the range might exist using the Bloom filter
    bool may_contain = false;
    for (int key = key1; key <= key2; ++key)
    {
        if (bloom_filter.MayContain(key))
        {
            may_contain = true;
            break;
        }
    }
    if (!may_contain)
    {
        return result;  // Skip scanning the SST
    }

    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0)
    {
        throw std::runtime_error("Failed to open SSTable file");
    }

    // Binary search to find the start of the range
    size_t left = 0;
    size_t right = num_entries - 1;
    size_t start_pos = num_entries;

    while (left <= right)
    {
        size_t mid = (left + right) / 2;
        off_t ofs = mid * sizeof(int) * 2;
        int key = readKey(fd, ofs);

        if (key < key1)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
            start_pos = mid;
        }
    }

    // Sequentially read from start position
    for (size_t i = start_pos; i < num_entries; i++)
    {
        off_t ofs = i * sizeof(int) * 2;
        int key = readKey(fd, ofs);
        if (key > key2) break;
        int value = readValue(fd, ofs + sizeof(int));
        result.push_back({key, value});
    }

    close(fd);
    return result;
}

int
SSTable::get(int key)
{
    // Use the Bloom filter to check if the key might exist
    static bool bloom_filter_message_printed =
        false;  // Flag to track message printing

    // Check the Bloom filter before accessing the SST
    if (!bloom_filter.MayContain(key))
    {
        if (!bloom_filter_message_printed)
        {
            printf(
                "Key %d is definitely not in %s (skipped using Bloom filter)\n",
                key, file_path.c_str());
            bloom_filter_message_printed = true;
        }
        return -1;  // Skip searching the SST
    }

    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) return -1;

    size_t left = 0;
    size_t right = num_entries - 1;

    while (left <= right)
    {
        size_t mid = left + (right - left) / 2;
        off_t ofs = mid * sizeof(int) * 2;
        int mid_key = readKey(fd, ofs);

        if (mid_key == key)
        {
            int value = readValue(fd, ofs + sizeof(int));
            close(fd);
            return value;
        }
        else if (mid_key < key)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    close(fd);
    return -1;  // Key not found
}

void
SSTable::write(const std::string& filename,
               const std::vector<std::pair<int, int>>& data)
{
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open())
    {
        throw std::runtime_error("Failed to create SST file: " + filename);
    }

    // Create a Bloom filter for the keys
    BloomFilter bloom_filter(data.size() * 10,
                             3);  // 10 bits per entry, 3 hash functions

    // Write each key-value pair to the file
    for (const auto& kv : data)
    {
        outFile.write(reinterpret_cast<const char*>(&kv.first),
                      sizeof(kv.first));
        outFile.write(reinterpret_cast<const char*>(&kv.second),
                      sizeof(kv.second));
        bloom_filter.Insert(kv.first);  // Add the key to the Bloom filter
    }

    outFile.close();

    // Serialize the Bloom filter to a separate file
    bloom_filter.SerializeToDisk(filename + ".filter");
}