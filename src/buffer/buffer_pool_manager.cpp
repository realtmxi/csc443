#include "buffer/buffer_pool_manager.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

BufferPoolManager::BufferPoolManager(size_t num_frames)
    : num_frames_(num_frames),
      next_page_id_(0),
      latch_(std::make_shared<std::mutex>()),
      replacer_(std::make_shared<ClockReplacer>(num_frames)) {
  for (frame_id_t i = 0; i < static_cast<frame_id_t>(num_frames); i++) {
    frames_.emplace_back(std::make_shared<FrameHeader>(i));
    free_frames_.push_back(i);
  }
}

BufferPoolManager::~BufferPoolManager() = default;

/* Fetch a page by its ID, loading it from disk if necessary. */
auto BufferPoolManager::FetchPage(page_id_t page_id) -> FrameHeader* 
{
  std::lock_guard<std::mutex> lock(*latch_);
  // Check if the page is already in the buffer pool
  if (page_table_.find(page_id) != page_table_.end()) {
    frame_id_t frame_id = page_table_[page_id];
    // Pin the frame
    replacer_->Pin(frame_id);
    return frames_[frame_id].get();
  }

  // If not in buffer, get a free frame or evict one
  frame_id_t frame_id;
  if (!free_frames_.empty()) {
    frame_id = free_frames_.front();
    free_frames_.pop_front();
  } else if (!replacer_->Victim(&frame_id)) {
    // No free frames and no victims available
    return nullptr; 
  }

  // Evict the victim frame if necessary
  auto &frame = frames_[frame_id];
  if (frame->is_dirty_) {
    std::ofstream file("data_" + std::to_string(frame->frame_id_) + ".bin", std::ios::binary);
    if (!file) 
      throw std::runtime_error("Failed to write dirty page to disk");
    file.write(frame->data_.data(), PAGE_SIZE);
    frame->is_dirty_ = false;
  }

  // Load the requested page into the evicted frame
  frame->Reset();
  std::ifstream file("data_" + std::to_string(page_id) + ".bin", std::ios::binary);
  if (file) {
    file.read(frame->data_.data(), PAGE_SIZE);
  } else {
    // File doesn't exist; treat as a new page
    frame->data_.resize(PAGE_SIZE, 0); // Zero-fill new pages
  }

  page_table_[page_id] = frame_id;
  return frame.get();
}

/* Create a new page and allocate a free frame for it. */
auto BufferPoolManager::NewPage() -> page_id_t {
  std::lock_guard<std::mutex> lock(*latch_);

  frame_id_t frame_id;
  if (!free_frames_.empty()) 
  {
    frame_id = free_frames_.front();
    free_frames_.pop_front();
  } else if (!replacer_->Victim(&frame_id)) 
  {
    // No free frames and no victim found
    return INVALID_PAGE_ID;
  }

  // Get the frame and reset its metadata
  auto &frame = frames_[frame_id];
  frame->Reset();

  // Assign a new page ID
  page_id_t page_id = next_page_id_++;
  page_table_[page_id] = frame_id;

  return page_id;
}

/* Delete a page from the buffer pool. */
auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool {
  std::lock_guard<std::mutex> lock(*latch_);

  if (page_table_.find(page_id) == page_table_.end()) {
    return false; // Page not found
  }

  frame_id_t frame_id = page_table_[page_id];
  auto &frame = frames_[frame_id];

  if (frame->pin_count_ > 0) {
    return false; // Page is pinned and cannot be deleted
  }

  // Remove from page table, mark as free
  page_table_.erase(page_id);
  replacer_->Pin(frame_id);
  free_frames_.push_back(frame_id);

  return true;
}

/* Write a page's content to disk if it is dirty. */
auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool {
  std::lock_guard<std::mutex> lock(*latch_);

  if (page_table_.find(page_id) == page_table_.end()) {
    return false; // Page not found
  }

  frame_id_t frame_id = page_table_[page_id];
  auto &frame = frames_[frame_id];

  if (frame->is_dirty_) 
  {
    std::ofstream file("data_" + std::to_string(page_id) + ".bin", std::ios::binary);
    if (!file) throw std::runtime_error("Failed to write page to disk");
    file.write(frame->data_.data(), PAGE_SIZE);
    frame->is_dirty_ = false;
  }

  return true;
}

/* Write all dirty pages to disk. */
void BufferPoolManager::FlushAllPages() {
  std::lock_guard<std::mutex> lock(*latch_);

  for (const auto &[page_id, frame_id] : page_table_) {
    FlushPage(page_id);
  }
}

/* Get the pin count for a given page. */
auto BufferPoolManager::GetPinCount(page_id_t page_id) -> std::optional<size_t> {
  std::lock_guard<std::mutex> lock(*latch_);

  if (page_table_.find(page_id) == page_table_.end()) {
    return std::nullopt; // Page not found
  }

  frame_id_t frame_id = page_table_[page_id];
  return frames_[frame_id]->pin_count_;
}

// FrameHeader implementation
FrameHeader::FrameHeader(frame_id_t frame_id)
    : frame_id_(frame_id), pin_count_(0), is_dirty_(false) {}

auto FrameHeader::GetData() const -> const char* {
  return data_.data();
}

void FrameHeader::Reset() {
  pin_count_ = 0;
  is_dirty_ = false;
  data_.clear();
  data_.resize(PAGE_SIZE);
}
