#ifndef BUFFER_POOL_MANAGER_H
#define BUFFER_POOL_MANAGER_H

#include <atomic>
#include <vector>
#include <memory>
#include <optional>
#include <list>
#include <unordered_map>
#include <shared_mutex>

#include "common/config.h"
#include "buffer/clock_replacer.h"
#include "third_party/murmur3/MurmurHash3.h"
#include "storage/disk/disk_manager.h"

class BufferPoolManager;

class FrameHeader {
  friend class BufferPoolManager;
 public:
  FrameHeader(frame_id_t frame_id);

 private:
  auto GetData() const-> const char*;
  void Reset();

  /* The frame index of the frame this header represents. */
  const frame_id_t frame_id_;
  
  /* The number of pins on the frame. */
  std::atomic<size_t> pin_count_;

  /* The dirty flag. */
  bool is_dirty_;

  /* The actual data of the page. */
  std::vector<char> data_;
  
  /* Page ID corresponding to the frame. */
  page_id_t page_id_;

};

/**
 * The buffer pool is responsible for moving physical pages of data back and 
 * forth from buffers in main memory to persistent storage. It also behaves as
 * a cache, keeping frequently used pages in memory for faster access, and 
 * evicting unused or cold pages back out to storage.
 */
class BufferPoolManager {
 public:
  BufferPoolManager(size_t num_frames);
  ~BufferPoolManager();

  auto Size() const -> size_t;
  auto FetchPage(page_id_t page_id) -> FrameHeader*; 
  auto NewPage() -> page_id_t;
  auto DeletePage(page_id_t page_id) -> bool;
  auto FlushPage(page_id_t page_id) -> bool;
  void FlushAllPages();
  auto GetPinCount(page_id_t page_id) -> std::optional<size_t>;
  auto SetPoolSize(size_t new_size) -> bool;
  static auto HashKey(const void *key_data, int len, uint32_t seed = 0) -> uint32_t;

 private:
  /** The number of frames in the buffer bool. */
  const size_t num_frames_; 

  /** The next page ID to be allocated. */
  std::atomic<page_id_t> next_page_id_;

  /** The frame headers of the frames that this buffer pool manages. */
  std::vector<std::shared_ptr<FrameHeader>> frames_;

  /** The latch protecting the buffer pool's inner data structure. */
  std::shared_ptr<std::mutex> latch_;

  /** The page table that keeps track of the mapping between pages and buffer pool frames. */
  std::unordered_map<page_id_t, frame_id_t> page_table_;

  /** A list of free frames that do not hold any page's data. */
  std::list<frame_id_t> free_frames_;

  /** The clock replacer to find pages for eviction. */
  std::shared_ptr<ClockReplacer> replacer_;

  /** The disk manager to read and write pages to disk. */
  std::shared_ptr<DiskManager> disk_manager_;

  /** Helper function to allocate a frame. */
  auto AllocateFrame(frame_id_t *frame_id) -> bool;

};

#endif // BUFFER_POOL_MANAGER_H