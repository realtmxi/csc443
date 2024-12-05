#include "bloom_filter.h"
#include <fstream>
#include <functional>

// Constructor: Initializes the Bloom filter with a given number of bits and hash functions.
BloomFilter::BloomFilter(size_t num_bits, size_t num_hashes)
    : bit_array_(num_bits, false), num_hashes_(num_hashes), num_bits_(num_bits) {}

// Inserts a key into the Bloom filter by setting the corresponding bits.
void BloomFilter::Insert(int key) {
  for (size_t i = 0; i < num_hashes_; ++i) {
    size_t hash = Hash(key, i) % num_bits_;
    bit_array_[hash] = true;
  }
}

// Checks if a key might exist in the Bloom filter (returns false if definitely not present).
bool BloomFilter::MayContain(int key) const {
  for (size_t i = 0; i < num_hashes_; ++i) {
    size_t hash = Hash(key, i) % num_bits_;
    if (!bit_array_[hash]) return false;
  }
  return true;
}

// Hash function: Computes a hash value for the given key and seed.
size_t BloomFilter::Hash(int key, int seed) const {
  std::hash<int> hasher;
  return hasher(key ^ seed);
}

// Serializes the Bloom filter to a binary file for persistent storage.
void BloomFilter::SerializeToDisk(const std::string &filename) const {
  std::ofstream out_file(filename, std::ios::binary);
  out_file.write(reinterpret_cast<const char *>(&num_bits_), sizeof(num_bits_));
  out_file.write(reinterpret_cast<const char *>(&num_hashes_), sizeof(num_hashes_));
  for (bool bit : bit_array_) {
    out_file.write(reinterpret_cast<const char *>(&bit), sizeof(bit));
  }
  out_file.close();
}

// Deserializes the Bloom filter from a binary file and restores its state.
void BloomFilter::DeserializeFromDisk(const std::string &filename) {
  std::ifstream in_file(filename, std::ios::binary);
  in_file.read(reinterpret_cast<char *>(&num_bits_), sizeof(num_bits_));
  in_file.read(reinterpret_cast<char *>(&num_hashes_), sizeof(num_hashes_));
  bit_array_.resize(num_bits_);
  for (size_t i = 0; i < num_bits_; ++i) {
    bool bit;
    in_file.read(reinterpret_cast<char *>(&bit), sizeof(bit));
    bit_array_[i] = bit;
  }
  in_file.close();
}

void BloomFilter::Union(const BloomFilter& other) {
    if (num_bits_ != other.num_bits_ || num_hashes_ != other.num_hashes_) {
        throw std::invalid_argument("Cannot union Bloom filters with different sizes or hash functions");
    }

    for (size_t i = 0; i < num_bits_; ++i) {
        bit_array_[i] = bit_array_[i] || other.bit_array_[i];
    }
}