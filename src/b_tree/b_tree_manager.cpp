

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

    // check if page_id * PAGE_SIZE is within the file size
    file.seekg(0, std::ios::end);
    int file_size = file.tellg();
    if (page_id * PAGE_SIZE >= file_size)
    {
        return BTreePage();
    }

    // Use a byte buffer (char did not work for me)
    std::byte buffer[PAGE_SIZE];
    file.seekg(page_id * PAGE_SIZE);
    if (!file.good())
    {
        return BTreePage();
    }

    file.read(reinterpret_cast<char *>(buffer), PAGE_SIZE);
    file.close();

    std::byte *buffer_ptr = buffer;

    // Read in the page type (4 bytes)
    BTreePageType page_type =
        *reinterpret_cast<const BTreePageType *>(buffer_ptr);
    buffer_ptr += sizeof(BTreePageType);
    if (page_type == BTreePageType::INVALID_PAGE)
    {
        return BTreePage();
    }

    // Read in the size of the page (4 bytes)
    int size = *reinterpret_cast<const int *>(buffer_ptr);
    buffer_ptr += sizeof(int);
    if (size == 0)
    {
        return BTreePage();
    }

    // Read in the key-value pairs (8 bytes each pair)
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

    // Create a new BTreePage object
    BTreePage page(pairs);
    page.SetPageType(page_type);
    page.SetSize(size);
    page.SetPageId(page_id);

    return page;
}

BTreePage
BTreeManager::TraverseToKey(int key) const
{
    // Start at the root page
    BTreePage page = ReadPageFromDisk(0);
    while (!page.IsLeafPage())
    {
        // Fine the child of the root that leads us to the key and read it
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

    // Start at the root page
    BTreePage page = ReadPageFromDisk(0);
    while (!page.IsLeafPage())
    {
        printf("Page type: %d\n", page.GetPageType());
        // Find the child of the root that leads us to the start key and read it
        int child_page_id = page.FindChildPage(start_key);
        page = ReadPageFromDisk(child_page_id);

        // If the page is invalid, return an empty result
        if (page.GetPageType() == BTreePageType::INVALID_PAGE)
        {
            return result;
        }
    }

    // page is a leaf that contains the start key. Scan the range
    while (page.IsLeafPage())
    {
        printf("Scanning page %d\n", page.GetPageId());
        std::vector<std::pair<int, int>> pairs = page.Scan(start_key, end_key);
        result.insert(result.end(), pairs.begin(), pairs.end());

        // The leaf pages are stored consecutively on disk, so if we have not
        // reached a key greater than end_key, we can read the next page
        if (page.GetMaxKey() >= end_key)
        {
            break;
        }

        int next_page_id = page.GetPageId() + 1;
        page = ReadPageFromDisk(next_page_id);
    }

    return result;
}