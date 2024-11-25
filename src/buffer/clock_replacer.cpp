#include "buffer/clock_replacer.h"
#include "common/macros.h"

ClockReplacer::ClockReplacer(size_t num_pages) {
  frames_.resize(num_pages, {false, true}); // All frames start as pinned
}

ClockReplacer::~ClockReplacer() = default;

auto ClockReplacer::Victim(frame_id_t *frame_id) -> bool {
  std::lock_guard<std::shared_mutex> lock(latch_);
  size_t num_frames = frames_.size();
  size_t count = 0;

  while (count < num_frames)
  {
    auto &frame = frames_[clock_hand_];

    if (!frame.pinned)
    {
      if (frame.ref)
      {
        frame.ref = false;
      }
      else
      {
        frame.pinned = true;
        *frame_id = clock_hand_;
        clock_hand_ = (clock_hand_ + 1) % num_frames;
        return true;
      }
    }

    clock_hand_ = (clock_hand_ + 1) % num_frames;
    count++;
  }

  return false; // No victim found;
}

void ClockReplacer::Pin(frame_id_t frame_id) {
  CSC443_ASSSERT(frame_id < frames_.size(), "Frame id out of range.");
  std::lock_guard<std::shared_mutex> lock(latch_);
  frames_[frame_id].pinned = true;
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
  CSC443_ASSSERT(frame_id < frames_.size(), "Frame id out of range.");
  std::lock_guard<std::shared_mutex> lock(latch_);
  frames_[frame_id].pinned = false;
  frames_[frame_id].ref = true; // Set reference bit to true when unpinned
}

size_t ClockReplacer::Size() -> size_t {
  std::shared_lock<std::shared_mutex> lock(latch_);
  size_t size = 0;
  for (const auto &frame : frames_) {
    if (!frame.pinned) {
      ++size;
    }
  }
  return size;
}
