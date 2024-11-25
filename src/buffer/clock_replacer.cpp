#include "buffer/clock_replacer.h"
#include "common/macros.h"

ClockReplacer::ClockReplacer(size_t num_pages) {
  for (size_t i = 0; i < num_pages; i++) {
    frames_.push_back(std::make_tuple(false, false));
  }
}

ClockReplacer::~ClockReplacer() = default;

auto ClockReplacer::Victim(frame_id_t *frame_id) -> bool {
  if (Size() == 0) {
    return false;
  }
  std::lock_guard<std::shared_mutex> lock(latch_);
  while (true) {
    // ref1 is the reference bit, ref2 ensures that active pages are not evicted
    auto &[ref1, ref2] = frames_[clock_hand_];
    if (!ref1) { // Unpinned frame
      if (!ref2) { // Reference bit is false, 
        ref2 = false;

        ref1 = false;
        *frame_id = clock_hand_;
        return true;
      }
    }
      clock_hand_ = (clock_hand_ + 1) % frames_.size();
  }
}

void ClockReplacer::Pin(frame_id_t frame_id) {
  CSC443_ASSSERT(frame_id < frames_.size(), "Frame id out of range.");
  std::lock_guard<std::shared_mutex> lock(latch_);
  auto &[ref1, ref2] = frames_[frame_id];
  ref1 = false;
  ref2 = false;
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
  CSC443_ASSSERT(frame_id < frames_.size(), "Frame id out of range.");
  std::lock_guard<std::shared_mutex> lock(latch_);
  auto &[ref1, ref2] = frames_[frame_id];
  ref1 = true;
  ref2 = true;
}

size_t ClockReplacer::Size() -> size_t {
  std::shared_lock<std::shared_mutex> lock(latch_);
  size_t size = 0;
  for (auto &[ref1, ref2] : frames_) {
    size += ref1;
    }
  return size;
}