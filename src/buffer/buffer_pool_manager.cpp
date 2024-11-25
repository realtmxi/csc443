#include "buffer/buffer_pool_manager.h"

BufferPoolManager::BufferPoolManager(size_t num_frames, DiskManager *disk_manager)
    : num_frames_(num_frames),
      next_page_id_(0);
      latch_(std::make_shared<std::mutex>()),
      replacer_(std::make_shared<ClockReplacer>(num_frames)),
      disk_scheduler_(std::make_unique<DiskScheduler>(disk_manager)) {

  
      }