#include <atomic>
#include <vector>

#include <include/utils/config.h>

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
class BufferPoolManager {
 public:
  BufferPoolManager(size_t num_frames);
  ~BufferPoolManager();

  auto Size() const -> size_t;

 private:
  const size_t num_frames_;

};