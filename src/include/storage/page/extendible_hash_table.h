#pragma once

#include <vector>
#include <unordered_map>
#include <list>
#include <memory>
#include <mutex>
#include "third_party/murmur3/MurmurHash3.h"

template <typename KeyType, typename ValueType>
class ExtendibleHashTable {
 public:
  ExtendibleHashTable(size_t initial_buckets = 2);
  ~ExtendibleHashTable() = default;

  auto Find(const KeyType& key, ValueType& value) -> bool;
  auto Insert(const KeyType& key, const ValueType& value) -> bool;
  auto Remove(const KeyType& key) -> bool;
  void Expand();

 private:
  size_t global_depth_;
  std::vector<std::list<std::pair<KeyType, ValueType>>> directory_;
  std::mutex latch_;

  size_t HashKey(const KeyType& key) const;
  size_t GetIndex(const KeyType& key) const;
};

template <typename KeyType, typename ValueType>
auto ExtendibleHashTable<KeyType, ValueType>::HashKey(const KeyType& key) const -> size_t {
  std::string key_str = std::to_string(key);  // Convert key to string for hashing
  return MurmurHash::Hash(key_str);
}
