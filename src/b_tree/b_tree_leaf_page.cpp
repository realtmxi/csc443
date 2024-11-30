#include "b_tree_leaf_page.h"

#include <algorithm>
#include <fstream>

#include "../include/common/config.h"

BTreeLeafPage::BTreeLeafPage(const std::vector<std::pair<int, int>>& KVPairs)
{
    for (const auto& pair : KVPairs)
    {
        keys_.push_back(pair.first);
        values_.push_back(pair.second);
    }
    SetPageType(BTreePageType::LEAF_PAGE);
    SetSize(static_cast<int>(KVPairs.size()));
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
BTreeLeafPage::Get(int key) const
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
BTreeLeafPage::Scan(int key1, int key2) const
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

int
BTreeLeafPage::GetMaxKey() const
{
    return keys_.back();
}