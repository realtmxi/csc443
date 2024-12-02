#include "b_tree.h"

#include <vector>

#include "../include/common/config.h"
#include "b_tree_page.h"

BTree::BTree(const std::vector<std::pair<int, int>>& data)
{
    std::vector<int> max_keys;
    ConstructLeafPages(data, max_keys);
    ConstructInternalNodes(max_keys);
}

void
BTree::ConstructLeafPages(const std::vector<std::pair<int, int>>& data,
                          std::vector<int>& max_keys)
{
    std::vector<std::pair<int, int>> keys;
    for (const auto& pair : data)
    {
        keys.push_back(pair);
        if (keys.size() == MAX_PAGE_KV_PAIRS || &pair == &data.back())
        {
            BTreePage leaf_page(keys);
            leaf_page.SetPageType(BTreePageType::LEAF_PAGE);
            leaf_page.SetSize(keys.size());
            leaf_pages_.push_back(leaf_page);
            max_keys.push_back(leaf_page.GetMaxKey());
            keys.clear();
        }
    }
}

void
BTree::ConstructInternalNodes(std::vector<int>& max_keys)
{
    std::vector<std::vector<BTreePage>> internal_page_layers;
    std::vector<int> internal_node_max_keys;

    while (!max_keys.empty())
    {
        std::vector<BTreePage> internal_pages;

        for (size_t i = 0; i < max_keys.size(); i += MAX_PAGE_KV_PAIRS)
        {
            std::vector<std::pair<int, int>> keys;

            for (size_t j = i; j < i + MAX_PAGE_KV_PAIRS && j < max_keys.size();
                 j++)
            {
                keys.push_back({max_keys[j], -1});
            }

            BTreePage page(keys);
            page.SetPageType(BTreePageType::INTERNAL_PAGE);
            page.SetSize(keys.size());
            internal_pages.push_back(page);
            internal_node_max_keys.push_back(page.GetMaxKey());
        }

        internal_page_layers.push_back(internal_pages);
        max_keys = internal_node_max_keys;
        internal_node_max_keys.clear();
        if (max_keys.size() == 1)
        {
            break;
        }
    }

    // loop through the internal page layers and set the child page IDs
    int child_page_id = 0;
    // Loop through the layers of internal nodes in reverse order
    for (size_t i = internal_page_layers.size(); i > 0; i--)
    {
        std::vector<BTreePage> internal_pages = internal_page_layers[i - 1];
        for (auto page : internal_pages)
        {
            // Set the child page ids for the internal node
            for (size_t k = 0; k < static_cast<size_t>(page.GetSize()); k++)
            {
                child_page_id += 1;
                page.SetValueAtIdx(k, child_page_id);
            }

            internal_pages_.push_back(page);
        }
    }
}

void
BTree::SaveBTreeToDisk(const std::string& filename)
{
    // Save each internal page to disk.
    for (const auto& internal_page : internal_pages_)
    {
        internal_page.WriteToDisk(filename);
    }

    // Save each leaf page to disk.
    for (const auto& leaf_page : leaf_pages_)
    {
        leaf_page.WriteToDisk(filename);
    }
}
