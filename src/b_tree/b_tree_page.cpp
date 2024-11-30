#include "b_tree_page.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <vector>

#include "../include/common/config.h"

// Constructor
BTreePage::BTreePage()
    : page_type_(BTreePageType::INVALID_PAGE), size_(0), page_id_(-1)
{
}

// Destructor
BTreePage::~BTreePage() = default;

// Check if the page is a leaf page.
bool
BTreePage::IsLeafPage() const
{
    return page_type_ == BTreePageType::LEAF_PAGE;
}

// Check if the page is an internal page.
bool
BTreePage::IsInternalPage() const
{
    return page_type_ == BTreePageType::INTERNAL_PAGE;
}

// Set the type of the page.
void
BTreePage::SetPageType(BTreePageType page_type)
{
    page_type_ = page_type;
}

// Get the type of the page.
BTreePageType
BTreePage::GetPageType() const
{
    return page_type_;
}

// Set the size of the page.
void
BTreePage::SetSize(int size)
{
    size_ = size;
}

// Get the size of the page.
int
BTreePage::GetSize() const
{
    return size_;
}

// Set the page ID.
void
BTreePage::SetPageId(int page_id)
{
    page_id_ = page_id;
}

// Get the page ID.
int
BTreePage::GetPageId() const
{
    return page_id_;
}

void
BTreeLeafPage::WriteToDisk(const std::string& filename) const
{
    std::ofstream out_file(filename, std::ios::binary | std::ios::app);
    if (!out_file.is_open())
    {
        throw std::runtime_error("Failed to create BTreeLeafPage file: " +
                                 filename);
    }
    BTreePageType page_type = GetPageType();
    out_file.write(reinterpret_cast<const char*>(&page_type),
                   sizeof(page_type));

    int size = GetSize();
    out_file.write(reinterpret_cast<const char*>(&size), sizeof(size));

    for (int i = 0; i < GetSize(); i++)
    {
        out_file.write(reinterpret_cast<const char*>(&keys_[i]),
                       sizeof(keys_[i]));
        out_file.write(reinterpret_cast<const char*>(&values_[i]),
                       sizeof(values_[i]));
    }

    int padding = PAGE_SIZE - (GetSize() * 2 * sizeof(int) + 8);
    for (int i = 0; i < padding; i++)
    {
        out_file.write(reinterpret_cast<const char*>(&padding),
                       sizeof(padding));
    }

    out_file.close();
}


int
BTreePage::Get(int key) const
{
    auto it = std::lower_bound(keys_.begin(), keys_.end(), key);

    if (it != keys_.end() && *it == key)
    {
        size_t index = std::distance(keys_.begin(), it);
        return values_[index];
    }

    return -1;
}

std::vector<std::pair<int, int>>
BTreePage::Scan(int key1, int key2) const
{
    std::vector<std::pair<int, int>> result;

    auto it = std::lower_bound(keys_.begin(), keys_.end(), key1);
    while (it != keys_.end() && *it <= key2)
    {
        size_t index = std::distance(keys_.begin(), it);
        result.push_back({*it, values_[index]});
        ++it;
    }

    return result;
}