#include "b_tree_internal_page.h"
#include <algorithm>

template <typename KeyType, typename PageIdType>
void BTreeInternalPage<KeyType, PageIdType>::Init(int max_size) {
  SetPageType(BTreePageType::INTERNAL_PAGE);
  SetMaxSize(max_size);
  SetSize(0);
}

template <typename KeyType, typename PageIdType>
auto BTreeInternalPage<KeyType, PageIdType>::KeyAt(int index) const -> KeyType {
  return keys_[index];
}

template <typename KeyType, typename PageIdType>
void BTreeInternalPage<KeyType, PageIdType>::SetKeyAt(int index, const KeyType& key) {
  keys_[index] = key;
}

template <typename KeyType, typename PageIdType>
auto BTreeInternalPage<KeyType, PageIdType>::ValueAt(int index) const -> PageIdType {
  return child_page_ids_[index];
}

template <typename KeyType, typename PageIdType>
void BTreeInternalPage<KeyType, PageIdType>::SetValueAt(int index, const PageIdType& value) {
  child_page_ids_[index] = value;
}

template <typename KeyType, typename PageIdType>
auto BTreeInternalPage<KeyType, PageIdType>::Insert(const KeyType& key, const PageIdType& value) -> bool {
  if (GetSize() >= GetMaxSize()) {
     // Internal page is full
    return false;
  }

  auto pos = std::upper_bound(keys_.begin(), keys_.begin() + GetSize(), key);
  int index = pos - keys_.begin();

  keys_.insert(keys_.begin() + index, key);
  child_page_ids_.insert(child_page_ids_.begin() + index + 1, value);
  IncreaseSize(1);
  return true;
}

template <typename KeyType, typename PageIdType>
auto BTreeInternalPage<KeyType, PageIdType>::Remove(const KeyType& key) -> bool {
  auto pos = std::lower_bound(keys_.begin(), keys_.begin() + GetSize(), key);
  if (pos == keys_.begin() + GetSize() || *pos != key) {
     // Key not found
    return false;
  }

  int index = pos - keys_.begin();
  keys_.erase(keys_.begin() + index);
  child_page_ids_.erase(child_page_ids_.begin() + index + 1);
  DecreaseSize(1);
  return true;
}

template class BTreeInternalPage<int, int>;
template class BTreeInternalPage<std::string, int>;
