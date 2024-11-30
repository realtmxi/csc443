

#include "b_tree_manager.h"

#include <cstdio>
#include <fstream>

#include "../include/common/config.h"
#include "b_tree_internal_page.h"
#include "b_tree_leaf_page.h"
#include "b_tree_page.h"

BTreeManager::BTreeManager(const std::string &filename) : filename_(filename) {}

int
BTreeManager::Get(int key)
{
    BTreePage page = TraverseToKey(key);
    if (page.IsLeafPage())
    {
        auto &leaf_page = dynamic_cast<BTreeLeafPage &>(page);
        return leaf_page.Get(key);
    }
    return -1;
}

std::vector<std::pair<int, int>>
BTreeManager::Scan(int start_key, int end_key)
{
    return TraverseRange(start_key, end_key);
}

// void
// BTreeManager::Merge(const std::string &filename_to_merge)
// {
//     MergeBTreeFromFile(filename_to_merge);
// }

BTreePage
BTreeManager::ReadPageFromDisk(int page_id) const
{
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open B-tree file: " + filename_);
    }

    // Seek to the page ID * PAGE_SIZE, get from there to +PAGE_SIZE
    file.seekg(page_id * PAGE_SIZE);
    if (file.eof())
    {
        return BTreePage();
    }
    char buffer[PAGE_SIZE];
    file.read(buffer, PAGE_SIZE);

    file.close();

    // Use a pointer to buffer for pointer arithmetic
    const char *buffer_ptr = buffer;

    // Read the page type
    BTreePageType page_type =
        *reinterpret_cast<const BTreePageType *>(buffer_ptr);
    buffer_ptr += sizeof(BTreePageType);

    // Read the size
    int size = *reinterpret_cast<const int *>(buffer_ptr);
    buffer_ptr += sizeof(int);

    // The next values are key-value pairs or key-child pairs, both are the same
    // size
    int num_pairs = size;
    std::vector<std::pair<int, int>> pairs;
    for (int i = 0; i < num_pairs; i++)
    {
        int key = *reinterpret_cast<const int *>(buffer_ptr);
        buffer_ptr += sizeof(int);
        int value = *reinterpret_cast<const int *>(buffer_ptr);
        buffer_ptr += sizeof(int);
        pairs.push_back({key, value});
    }

    // Create a new internal or leaf page based on the type
    if (page_type == BTreePageType::INTERNAL_PAGE)
    {
        BTreeInternalPage internal_page(pairs);
        internal_page.SetPageId(page_id);
        printf("Read internal page from disk\n");
        return internal_page;
    }
    if (page_type == BTreePageType::LEAF_PAGE)
    {
        BTreeLeafPage leaf_page(pairs);
        leaf_page.SetPageId(page_id);
        printf("Read leaf page from disk\n");
        return leaf_page;
    }

    return BTreePage();
}

BTreePage
BTreeManager::TraverseToKey(int key) const
{
    BTreePage page = ReadPageFromDisk(0);
    while (!page.IsLeafPage())
    {
        auto &internal_page = dynamic_cast<BTreeInternalPage &>(page);
        int child_page_id = internal_page.GetChildPageId(key);
        page = ReadPageFromDisk(child_page_id);

        // If the page is invalid, return an empty page
        if (page.GetPageType() == BTreePageType::INVALID_PAGE)
        {
            return BTreePage();
        }
    }

    return page;
}

std::vector<std::pair<int, int>>
BTreeManager::TraverseRange(int start_key, int end_key) const
{
    std::vector<std::pair<int, int>> result;
    BTreePage page = ReadPageFromDisk(0);
    while (!page.IsLeafPage())
    {
        auto &internal_page = dynamic_cast<BTreeInternalPage &>(page);
        int child_page_id = internal_page.GetChildPageId(start_key);
        page = ReadPageFromDisk(child_page_id);

        // If the page is invalid, return an empty result
        if (page.GetPageType() == BTreePageType::INVALID_PAGE)
        {
            return result;
        }
    }

    // Traverse the leaf pages and collect the key-value pairs
    while (page.IsLeafPage())
    {
        auto &leaf_page = dynamic_cast<BTreeLeafPage &>(page);
        std::vector<std::pair<int, int>> pairs =
            leaf_page.Scan(start_key, end_key);
        result.insert(result.end(), pairs.begin(), pairs.end());

        // Get the next page ID
        int next_page_id = leaf_page.GetPageId() + 1;

        // Read the next page
        page = ReadPageFromDisk(next_page_id);
    }

    return result;
}