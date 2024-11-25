#include <atomic>
#include <vector>
#include <memory>
#include <optional>
#include <include/common/config.h>

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

  std::vector<char> data_;

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
  auto NewPage() -> page_id_t;
  auto DeletePage(page_id_t page_id) -> bool;
  auto FlushPage(page_id_t page_id) -> bool;
  void FlushAllPages();
  auto GetPinCount(page_id_t page_id) -> std::optional<size_t>;


 private:
  /** The number of frames in the buffer bool. */
  const size_t num_frames_; 

  /** The next page ID to be allocated. */
  std::atomic<page_id_t> next_page_id_;

  /** The frame headers of the frames that this buffer pool manages. */
  std::vector<std::shared_ptr<FrameHeader>> frames_;

  /** The page table that keeps track of the mapping between pages and buffer pool frames. */

};