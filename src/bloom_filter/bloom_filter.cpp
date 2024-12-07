#include "bloom_filter.h"

#include <fstream>
#include <functional>

#include "../include/common/config.h"

// Constructor: Initializes the Bloom filter with a given number of bits
BloomFilter::BloomFilter(size_t num_bits)
    : bit_array_(num_bits, false), num_bits_(num_bits)
{
    // optimal hash functions is k = (m/n) * ln(2), where m is the
    // number of bits and n is the number of keys.
    num_hashes_ = static_cast<size_t>(std::round(
        (num_bits_ / static_cast<double>(MAX_KEYS_IN_MEMTABLE)) * std::log(2)));
}

BloomFilter::BloomFilter(const std::string &filename)
{
    DeserializeFromDisk(filename);
}

// Inserts a key into the Bloom filter by setting the corresponding bits.
void
BloomFilter::Insert(int key)
{
    for (size_t i = 0; i < num_hashes_; ++i)
    {
        size_t hash = Hash(key, i) % num_bits_;
        bit_array_[hash] = true;
    }
}

// Checks if a key might exist in the Bloom filter (returns false if definitely
// not present).
bool
BloomFilter::MayContain(int key) const
{
    for (size_t i = 0; i < num_hashes_; ++i)
    {
        size_t hash = Hash(key, i) % num_bits_;
        if (!bit_array_[hash]) return false;
    }
    return true;
}

// Hash function: Computes a hash value for the given key and seed.
size_t
BloomFilter::Hash(int key, int seed) const
{
    std::hash<int> hasher;
    return hasher(key ^ seed);
}
void
BloomFilter::SerializeToDisk(const std::string &filename) const
{
    std::ofstream out_file(filename, std::ios::binary);
    out_file.write(reinterpret_cast<const char *>(&num_bits_),
                   sizeof(num_bits_));
    out_file.write(reinterpret_cast<const char *>(&num_hashes_),
                   sizeof(num_hashes_));

    size_t num_bytes = (num_bits_ + 7) / 8;  // Round up to the nearest byte
    std::vector<uint8_t> byte_array(num_bytes, 0);

    for (size_t i = 0; i < num_bits_; ++i)
    {
        if (bit_array_[i])
        {
            byte_array[i / 8] |= (1 << (i % 8));
        }
    }

    out_file.write(reinterpret_cast<const char *>(byte_array.data()),
                   num_bytes);
    out_file.close();
}

void
BloomFilter::DeserializeFromDisk(const std::string &filename)
{
    std::ifstream in_file(filename, std::ios::binary);
    in_file.read(reinterpret_cast<char *>(&num_bits_), sizeof(num_bits_));
    in_file.read(reinterpret_cast<char *>(&num_hashes_), sizeof(num_hashes_));

    size_t num_bytes = (num_bits_ + 7) / 8;  // Round up to the nearest byte
    std::vector<uint8_t> byte_array(num_bytes, 0);
    in_file.read(reinterpret_cast<char *>(byte_array.data()), num_bytes);

    bit_array_.resize(num_bits_);
    for (size_t i = 0; i < num_bits_; ++i)
    {
        bit_array_[i] = (byte_array[i / 8] & (1 << (i % 8))) != 0;
    }

    in_file.close();
}

void
BloomFilter::Union(const BloomFilter &other)
{
    if (num_bits_ != other.num_bits_ || num_hashes_ != other.num_hashes_)
    {
        throw std::invalid_argument(
            "Cannot union Bloom filters with different sizes or hash "
            "functions");
    }

    for (size_t i = 0; i < num_bits_; ++i)
    {
        bit_array_[i] = bit_array_[i] || other.bit_array_[i];
    }
}