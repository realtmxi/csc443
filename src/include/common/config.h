#include <cstdint>

using frame_id_t = int32_t; // frame id type
using page_id_t = int32_t; // page id type

static constexpr int BUFFER_POOL_SIZE = 128; // size of buffer pool
static constexpr int PAGE_SIZE = 4096; // size of data page in byte
static constexpr int BUCKET_SIZE = 50; // size of extendible hash bucket6

constexpr page_id_t INVALID_PAGE_ID = static_cast<page_id_t>(-1);
