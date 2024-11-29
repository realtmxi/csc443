#pragma once

#include "buffer/buffer_pool_manager.h"
#include "page/b_tree_page.h"
#include "page/b_tree_leaf_page.h"
#include "page/b_tree_internal_page.h"


template <typename KeyType, typename ValueType, typename KeyComparator>
class BTreeIndex {
 public:
  BTreeIndex(BufferPoolManager* buffer_pool_manager, int max_leaf_size, int max_internal_size);

  auto Insert(const KeyType& key, const ValueType& value) -> bool;
  auto Delete(const KeyType& key) -> bool;
  auto Find(const KeyType& key, ValueType& value) -> bool;

 private:
  auto FindLeafPage(const KeyType& key, bool is_insert = false) -> BTreeLeafPage<KeyType, ValueType>*;
  void SplitLeafPage(BTreeLeafPage<KeyType, ValueType>* leaf_page);
  void SplitInternalPage(BTreeInternalPage<KeyType, int>* internal_page);

  int root_page_id_;
  BufferPoolManager* buffer_pool_manager_;
  int max_leaf_size_;
  int max_internal_size_;
};
