#include "b_tree_leaf_page.h"
#include <algorithm>

template <typename KeyType, typename ValueType>
void BTreeLeafPage<KeyType, ValueType>::Init(int max_size) {
  SetPageType(BTreePageType::LEAF_PAGE);
  SetMaxSize(max_size);
  SetSize(0);
  next_page_id_ = INVALID_PAGE_ID;
}

template <typename KeyType, typename ValueType>
auto BTreeLeafPage<KeyType, ValueType>::GetNextPageId() const -> int {
  return next_page_id_;
}

template <typename KeyType, typename ValueType>
void BTreeLeafPage<KeyType, ValueType>::SetNextPageId(int next_page_id) {
  next_page_id_ = next_page_id;
}

template <typename KeyType, typename ValueType>
auto BTreeLeafPage<KeyType, ValueType>::KeyAt(int index) const -> KeyType {
  return keys_[index];
}

template <typename KeyType, typename ValueType>
void BTreeLeafPage<KeyType, ValueType>::SetKeyAt(int index, const KeyType& key) {
  keys_[index] = key;
}

template <typename KeyType, typename ValueType>
auto BTreeLeafPage<KeyType, ValueType>::ValueAt(int index) const -> ValueType {
  return values_[index];
}

template <typename KeyType, typename ValueType>
void BTreeLeafPage<KeyType, ValueType>::SetValueAt(int index, const ValueType& value) {
  values_[index] = value;
}

template <typename KeyType, typename ValueType>
auto BTreeLeafPage<KeyType, ValueType>::Insert(const KeyType& key, const ValueType& value) -> bool {
  if (GetSize() >= GetMaxSize()) 
  {
    // Leaf page is full
    return false;
  }

  auto pos = std::lower_bound(keys_.begin(), keys_.begin() + GetSize(), key);
  int index = pos - keys_.begin();

  keys_.insert(keys_.begin() + index, key);
  values_.insert(values_.begin() + index, value);
  IncreaseSize(1);
  return true;
}

template <typename KeyType, typename ValueType>
auto BTreeLeafPage<KeyType, ValueType>::Remove(const KeyType& key) -> bool {
  auto pos = std::lower_bound(keys_.begin(), keys_.begin() + GetSize(), key);
  if (pos == keys_.begin() + GetSize() || *pos != key) {
    return false; // Key not found
  }

  int index = pos - keys_.begin();
  keys_.erase(keys_.begin() + index);
  values_.erase(values_.begin() + index);
  DecreaseSize(1);
  return true;
}

template class BTreeLeafPage<int, int>;
template class BTreeLeafPage<std::string, int>;
