#include "b_tree_page.h"

#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../include/common/config.h"

// Constructor for an empty, invalid page.
BTreePage::BTreePage()
    : page_type_(BTreePageType::INVALID_PAGE), size_(0), page_id_(-1)
{
}

// Constructor to build a page from a list of key-value pairs.
BTreePage::BTreePage(const std::vector<std::pair<int, int>>& key_value_pairs)
    : page_type_(BTreePageType::INVALID_PAGE),
      size_(key_value_pairs.size()),
      page_id_(-1)
{
    for (const auto& pair : key_value_pairs)
    {
        keys_.push_back(pair.first);
        values_.push_back(pair.second);
    }
}

bool
BTreePage::IsLeafPage() const
{
    return page_type_ == BTreePageType::LEAF_PAGE;
}

bool
BTreePage::IsInternalPage() const
{
    return page_type_ == BTreePageType::INTERNAL_PAGE;
}

void
BTreePage::SetPageType(BTreePageType page_type)
{
    page_type_ = page_type;
}

BTreePageType
BTreePage::GetPageType() const
{
    return page_type_;
}

void
BTreePage::SetSize(int size)
{
    size_ = size;
}

int
BTreePage::GetSize() const
{
    return size_;
}

void
BTreePage::SetPageId(int page_id)
{
    page_id_ = page_id;
}

int
BTreePage::GetPageId() const
{
    return page_id_;
}

void
BTreePage::WriteToDisk(const std::string& filename) const
{
    std::ofstream out_file(filename, std::ios::binary);
    if (!out_file.is_open())
    {
        throw std::runtime_error("Failed to create BTree file: " + filename);
    }

    // Write the page type and size to the binary file
    BTreePageType page_type = GetPageType();
    out_file.write(reinterpret_cast<const char*>(&page_type),
                   sizeof(page_type));

    int size = GetSize();
    out_file.write(reinterpret_cast<const char*>(&size), sizeof(size));

    // Write the key-value pairs to the binary file
    for (int i = 0; i < GetSize(); i++)
    {
        out_file.write(reinterpret_cast<const char*>(&keys_[i]),
                       sizeof(keys_[i]));
        out_file.write(reinterpret_cast<const char*>(&values_[i]),
                       sizeof(values_[i]));
    }

    // Pad the rest of the page with zeros.
    int num_bytes_to_pad = PAGE_SIZE - (sizeof(page_type) + sizeof(size) +
                                        GetSize() * (sizeof(int) * 2));
    char zero = 0;
    for (int i = 0; i < num_bytes_to_pad; i++)
    {
        out_file.write(&zero, sizeof(zero));
    }

    out_file.flush();
    out_file.close();
}

int
BTreePage::Get(int key) const
{
    // Perform a binary search for the key
    auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
    if (it != keys_.end() && *it == key)
    {
        size_t index = std::distance(keys_.begin(), it);
        return values_[index];
    }

    return -1;
}

int
BTreePage::FindChildPage(int key) const
{
    // Perform a binary search for the closest key greater than the input key.
    // If the key is greater than all keys, return the last child page ID.
    auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
    size_t index = std::distance(keys_.begin(), it);
    return values_[index];
}

std::vector<std::pair<int, int>>
BTreePage::Scan(int key1, int key2) const
{
    std::vector<std::pair<int, int>> result;

    // for now, just read the keys in order and stop when we reach key2 or
    // greater
    for (size_t i = 0; i < keys_.size(); i++)
    {
        if (keys_[i] >= key1 && keys_[i] <= key2)
        {
            printf("Adding key %d\n", keys_[i]);
            result.push_back({keys_[i], values_[i]});
        }

        if (keys_[i] > key2)
        {
            break;
        }
    }

    return result;
}

int
BTreePage::GetMaxKey() const
{
    return keys_.back();
}