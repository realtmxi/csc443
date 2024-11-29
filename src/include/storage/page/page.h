#pragma once

#include <atomic>
#include <cstring>
#include <vector>
#include <shared_mutex>

constexpr size_t PAGE_SIZE = 4096;
constexpr int INVALID_PAGE_ID = -1;

class Page {
 public:
  Page();
  ~Page() = default;

  // Accessors for page data
  auto GetData() -> char*;
  auto GetPageId() const -> int;
  auto GetPinCount() const -> int;
  auto IsDirty() const -> bool;

  // Setters
  void SetPageId(int page_id);
  void SetDirty(bool is_dirty);
  void ResetMemory();

  // Latching mechanisms
  void WLatch();
  void WUnlatch();
  void RLatch();
  void RUnlatch();

 private:
  std::vector<char> data_;       // Page data
  int page_id_;                  // Page ID
  std::atomic<int> pin_count_;   // Pin count
  std::atomic<bool> is_dirty_;   // Dirty flag
  std::shared_mutex rwlatch_;    // Read-write latch
};
