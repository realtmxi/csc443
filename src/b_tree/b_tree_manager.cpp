

#include "b_tree_manager.h"

#include <cstdio>
#include <fstream>

#include "../include/common/config.h"
#include "b_tree_page.h"

BTreeManager::BTreeManager(const std::string &filename) : filename_(filename) {}

int
BTreeManager::Get(int key)
{
    BTreePage page = TraverseToKey(key);
    if (page.IsLeafPage())
    {
        return page.Get(key);
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

    std::byte buffer[PAGE_SIZE];

    printf("Reading page from disk for pageid: %d\n", page_id);
    file.seekg(page_id * PAGE_SIZE);
    file.read(reinterpret_cast<char *>(buffer), PAGE_SIZE);
    file.close();

    std::byte *buffer_ptr = buffer;

    BTreePageType page_type =
        *reinterpret_cast<const BTreePageType *>(buffer_ptr);
    buffer_ptr += sizeof(BTreePageType);

    if (page_type == BTreePageType::INVALID_PAGE)
    {
        return BTreePage();
    }

    int size = *reinterpret_cast<const int *>(buffer_ptr);
    buffer_ptr += sizeof(int);
    if (size == 0)
    {
        return BTreePage();
    }

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

    if (pairs.empty())
    {
        return BTreePage();
    }

    BTreePage page(pairs);
    page.SetPageType(page_type);
    page.SetSize(size);
    page.SetPageId(page_id);

    return page;
}

BTreePage
BTreeManager::TraverseToKey(int key) const
{
    BTreePage page = ReadPageFromDisk(0);
    while (!page.IsLeafPage())
    {
        int child_page_id = page.FindChildPage(key);
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
        int child_page_id = page.Get(start_key);
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
        std::vector<std::pair<int, int>> pairs = page.Scan(start_key, end_key);
        result.insert(result.end(), pairs.begin(), pairs.end());

        // Get the next page ID
        int next_page_id = page.GetPageId() + 1;

        // Read the next page
        page = ReadPageFromDisk(next_page_id);
    }

    return result;
}