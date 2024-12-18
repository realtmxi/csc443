#include <climits>  // INT_MAX
#include <cstdint>

using frame_id_t = int32_t;  // frame id type
using page_id_t = int32_t;   // page id type

static constexpr int BUFFER_POOL_SIZE = 128;  // size of buffer pool
static constexpr int PAGE_SIZE = 4096;        // size of data page in byte
static constexpr int BUCKET_SIZE = 50;        // size of extendible hash bucket6
static constexpr int MAX_PAGE_KV_PAIRS = (PAGE_SIZE - 16) / 8;
static constexpr int MEMTABLE_SIZE = 1024 * 1024;  // 1MB memtable size
static constexpr int MAX_KEYS_IN_MEMTABLE = MEMTABLE_SIZE / 8;
static constexpr int BLOOM_FILTER_BITS = MAX_KEYS_IN_MEMTABLE * 8;
static constexpr int MAX_BUFFER_POOL_SIZE =
    10 * 1024 * 1024 / PAGE_SIZE;  // 10MB buffer pool size

constexpr page_id_t INVALID_PAGE_ID = static_cast<page_id_t>(-1);
