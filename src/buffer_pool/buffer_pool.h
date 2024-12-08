#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>

#include "../b_tree/b_tree_page.h"

class BufferPool
{
   public:
    explicit BufferPool(size_t max_number_of_pages);
    void EvictAllPages();

    // dependency inject the function to load a page from disk
    BTreePage GetPageFromId(
        const std::string &filename, int page_id,
        const std::function<BTreePage(int, const std::string &)>
            &loadPageFromDisk);

   private:
    size_t max_number_of_pages_;

    // The LRU list is a list of pairs of the filename+id and the BTreePage
    std::list<std::pair<std::string, BTreePage>> lru_list_;

    // The page_table_ is a map of the filename+id to an iterator in the LRU
    // list. This allows us to quickly find the location of a page in the LRU
    // list.
    std::unordered_map<std::string,
                       std::list<std::pair<std::string, BTreePage>>::iterator>
        page_table_;
    void EvictPage();
    void AddPageToPool(const std::string &filename, int page_id,
                       const BTreePage &page);
};

#endif