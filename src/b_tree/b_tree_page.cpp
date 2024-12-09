#include "b_tree_page.h"

#include <fcntl.h>  // For open, O_DIRECT
#include <unistd.h>
#include <unistd.h>  // For close, pwrite

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>  // For posix_memalign
#include <cstring>  // For memset
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../config.h"

// Constructor for an empty, invalid page.
BTreePage::BTreePage()
    : page_type_(BTreePageType::INVALID_PAGE), size_(0), page_id_(-1)
{
}

// Constructor to build a page from a list of key-value pairs.
BTreePage::BTreePage(const std::vector<std::pair<int, int>> &key_value_pairs)
    : page_type_(BTreePageType::INVALID_PAGE),
      size_(key_value_pairs.size()),
      page_id_(-1)
{
    for (const auto &pair : key_value_pairs)
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
    // if we set it to internal node, we need to add one more value to point
    // to the right most child
    if (page_type == BTreePageType::INTERNAL_PAGE)
    {
        values_.push_back(values_.back() + 1);
    }
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
BTreePage::WriteToDisk(const std::string &filename) const
{
    // Open the file with O_DIRECT and O_CREAT for writing
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666);
    if (fd < 0)
    {
        throw std::runtime_error("Failed to create BTree file: " + filename);
    }

    #ifdef __APPLE__
        // macOS-specific code for disabling caching
        fcntl(fd, F_NOCACHE, 1);
    #elif defined(__linux__)
        posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
    #endif
    
    // Allocate aligned memory for the buffer
    void *aligned_buffer;
    if (posix_memalign(&aligned_buffer, PAGE_SIZE, PAGE_SIZE) != 0)
    {
        close(fd);
        throw std::runtime_error("Failed to allocate aligned memory");
    }
    std::memset(aligned_buffer, 0, PAGE_SIZE);

    std::byte *buffer_ptr = static_cast<std::byte *>(aligned_buffer);
    int bytes_written = 0;

    // Write the page type to the buffer
    BTreePageType page_type = GetPageType();
    std::memcpy(buffer_ptr, &page_type, sizeof(page_type));
    buffer_ptr += sizeof(page_type);
    bytes_written += sizeof(page_type);

    // Write the size of the page to the buffer
    int size = GetSize();
    std::memcpy(buffer_ptr, &size, sizeof(size));
    buffer_ptr += sizeof(size);
    bytes_written += sizeof(size);

    // Write the key-value pairs to the buffer
    for (int i = 0; i < GetSize(); i++)
    {
        std::memcpy(buffer_ptr, &keys_[i], sizeof(keys_[i]));
        buffer_ptr += sizeof(keys_[i]);
        bytes_written += sizeof(keys_[i]);

        std::memcpy(buffer_ptr, &values_[i], sizeof(values_[i]));
        buffer_ptr += sizeof(values_[i]);
        bytes_written += sizeof(values_[i]);
    }

    // Write the child page ID for internal nodes (if applicable)
    if (page_type == BTreePageType::INTERNAL_PAGE)
    {
        std::memcpy(buffer_ptr, &values_.back(), sizeof(values_.back()));
        buffer_ptr += sizeof(values_.back());
        bytes_written += sizeof(values_.back());
    }

    // Zero-pad the rest of the buffer to PAGE_SIZE
    unsigned int num_bytes_to_pad = PAGE_SIZE - bytes_written;
    std::memset(buffer_ptr, 0, num_bytes_to_pad);

    // Write the aligned buffer to disk
    ssize_t written =
        pwrite(fd, aligned_buffer, PAGE_SIZE, lseek(fd, 0, SEEK_END));
    if (written != PAGE_SIZE)
    {
        free(aligned_buffer);
        close(fd);
        throw std::runtime_error("Failed to write the full page to disk");
    }

    // Clean up
    free(aligned_buffer);
    close(fd);
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
    auto it = std::lower_bound(keys_.begin(), keys_.end(), key);

    // take the child of the first key that is equal to or greater than the
    // given
    size_t index = it - keys_.begin();
    if (it != keys_.end() && key <= *it)
    {
        return values_[index];
    }

    return -1;
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

int
BTreePage::GetMinKey() const
{
    return keys_.front();
}

std::vector<std::pair<int, int>>
BTreePage::GetKeyValues() const
{
    std::vector<std::pair<int, int>> result;
    for (size_t i = 0; i < keys_.size(); i++)
    {
        result.push_back({keys_[i], values_[i]});
    }

    return result;
}

void
BTreePage::SetValueAtIdx(int idx, int value)
{
    if (idx == -1)
    {
        values_.push_back(value);
        return;
    }
    values_[idx] = value;
}