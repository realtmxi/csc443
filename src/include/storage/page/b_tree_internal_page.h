#pragma once

#include <vector>
#include <utility>
#include "b_tree_page.h"

/**
 * Internal page for B-Tree.
 * Stores keys and child page pointers.
 */
template <typename KeyType, typename PageIdType>
class BTreeInternalPage : public BTreePage {
 public:
  BTreeInternalPage() = delete;
  ~BTreeInternalPage() = default;

  void Init(int max_size);

  auto KeyAt(int index) const -> KeyType;
  void SetKeyAt(int index, const KeyType& key);

  auto ValueAt(int index) const -> PageIdType;
  void SetValueAt(int index, const PageIdType& value);

  auto Insert(const KeyType& key, const PageIdType& value) -> bool;
  auto Remove(const KeyType& key) -> bool;

 private:
  std::vector<KeyType> keys_;
  std::vector<PageIdType> child_page_ids_;
};
