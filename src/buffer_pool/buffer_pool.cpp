

#include "buffer_pool.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

BufferPool::BufferPool(size_t max_number_of_pages)
    : max_number_of_pages_(max_number_of_pages)
{
}

void
BufferPool::EvictAllPages()
{
    lru_list_.clear();
    page_table_.clear();
}

BTreePage
BufferPool::GetPageFromId(
    const std::string &filename, int page_id,
    const std::function<BTreePage(int, const std::string &)> &loadPageFromDisk)
{
    std::string key = filename + std::to_string(page_id);
    auto it = page_table_.find(key);

    // If the page is in the buffer pool, move it to the front of the LRU list
    if (it != page_table_.end())
    {
        auto page = it->second->second;
        lru_list_.erase(it->second);
        lru_list_.push_front({key, page});
        page_table_[key] = lru_list_.begin();
        return page;
    }

    // If the page is not in the buffer pool, load it from disk
    BTreePage page = loadPageFromDisk(page_id, filename);

    // If the buffer pool is full, evict the least recently used page
    if (lru_list_.size() == max_number_of_pages_)
    {
        EvictPage();
    }

    // Add the new page to the buffer pool
    AddPageToPool(filename, page_id, page);

    return page;
}

void
BufferPool::EvictPage()
{
    auto last = lru_list_.end();
    --last;
    page_table_.erase(last->first);
    lru_list_.pop_back();
}

void
BufferPool::AddPageToPool(const std::string &filename, int page_id,
                          const BTreePage &page)
{
    std::string key = filename + std::to_string(page_id);
    lru_list_.push_front({key, page});
    page_table_[key] = lru_list_.begin();
}