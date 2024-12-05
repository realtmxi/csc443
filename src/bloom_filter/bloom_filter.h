#pragma once // ensure the header file is included only once during compilation.
#include <vector>
#include <cmath>
#include <string>

class BloomFilter {
 public:
  BloomFilter(size_t num_bits, size_t num_hashes);
  void Insert(int key);
  bool MayContain(int key) const;
  void SerializeToDisk(const std::string &filename) const;
  void DeserializeFromDisk(const std::string &filename);
  void Union(const BloomFilter& other);

 private:
  size_t Hash(int key, int seed) const;
  std::vector<bool> bit_array_;
  size_t num_hashes_;
  size_t num_bits_;
};