#include "b_tree_page.h"

BTreePage::BTreePage() : page_type_(BTreePageType::INVALID_PAGE), size_(0), max_size_(0), parent_page_id_(INVALID_PAGE_ID) {}

auto BTreePage::IsLeafPage() const -> bool {
  return page_type_ == BTreePageType::LEAF_PAGE;
}

auto BTreePage::IsRootPage() const -> bool {
  return parent_page_id_ == INVALID_PAGE_ID;
}

void BTreePage::SetPageType(BTreePageType page_type) {
  page_type_ = page_type;
}

auto BTreePage::GetPageType() const -> BTreePageType {
  return page_type_;
}

auto BTreePage::GetSize() const -> int {
  return size_;
}

void BTreePage::SetSize(int size) {
  size_ = size;
}

void BTreePage::IncreaseSize(int amount) {
  size_ += amount;
}

void BTreePage::DecreaseSize(int amount) {
  size_ -= amount;
}

auto BTreePage::GetMaxSize() const -> int {
  return max_size_;
}

void BTreePage::SetMaxSize(int max_size) {
  max_size_ = max_size;
}

auto BTreePage::GetParentPageId() const -> int {
  return parent_page_id_;
}

void BTreePage::SetParentPageId(int parent_page_id) {
  parent_page_id_ = parent_page_id;
}
