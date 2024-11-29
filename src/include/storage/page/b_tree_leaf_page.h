#pragma once

#include <vector>
#include <utility>
#include <string>
#include "b_tree_page.h"

/**
 * Leaf page for B-Tree.
 * Stores keys and associated values.
 */
template <typename KeyType, typename ValueType>
class BTreeLeafPage : public BTreePage {
 public:
  BTreeLeafPage() = delete;
  ~BTreeLeafPage() = default;

  void Init(int max_size);

  auto GetNextPageId() const -> int;
  void SetNextPageId(int next_page_id);

  auto KeyAt(int index) const -> KeyType;
  void SetKeyAt(int index, const KeyType& key);

  auto ValueAt(int index) const -> ValueType;
  void SetValueAt(int index, const ValueType& value);

  auto Insert(const KeyType& key, const ValueType& value) -> bool;
  auto Remove(const KeyType& key) -> bool;

 private:
  int next_page_id_;
  std::vector<KeyType> keys_;
  std::vector<ValueType> values_;
};
