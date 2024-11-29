#pragma once

#include <memory>
#include "page.h"
#include "buffer_pool_manager.h"

class BufferPoolManager;

class ReadPageGuard {
 public:
  ReadPageGuard();
  explicit ReadPageGuard(Page* page, BufferPoolManager* bpm);
  ReadPageGuard(const ReadPageGuard&) = delete;
  ReadPageGuard(ReadPageGuard&& other) noexcept;
  ~ReadPageGuard();

  auto GetData() const -> const char*;

 private:
  Page* page_;
  BufferPoolManager* bpm_;
  bool is_valid_;
};

class WritePageGuard {
 public:
  WritePageGuard();
  explicit WritePageGuard(Page* page, BufferPoolManager* bpm);
  WritePageGuard(const WritePageGuard&) = delete;
  WritePageGuard(WritePageGuard&& other) noexcept;
  ~WritePageGuard();

  auto GetDataMut() -> char*;

 private:
  Page* page_;
  BufferPoolManager* bpm_;
  bool is_valid_;
};
