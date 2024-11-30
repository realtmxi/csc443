#include "b_tree_internal_page.h"

#include <fstream>
#include <stdexcept>
#include <vector>

#include "../include/common/config.h"

BTreeInternalPage::BTreeInternalPage(
    const std::vector<std::pair<int, int>>& key_child_pairs)
{
    for (const auto& pair : key_child_pairs)
    {
        keys_.push_back(pair.first);
        child_page_ids_.push_back(pair.second);
    }
    SetPageType(BTreePageType::INTERNAL_PAGE);
    SetSize(static_cast<int>(key_child_pairs.size()));
}

void
BTreeInternalPage::WriteToDisk(const std::string& filename) const
{
    // Write in order: PageType (4) | CurrentSize (4) | Key1 (4) | ChildPageId1
    // (4) | ...

    std::ofstream out_file(filename, std::ios::binary | std::ios::app);
    if (!out_file.is_open())
    {
        throw std::runtime_error("Failed to create BTreeInternalPage file: " +
                                 filename);
    }

    // Write page type
    BTreePageType page_type = GetPageType();
    out_file.write(reinterpret_cast<const char*>(&page_type),
                   sizeof(page_type));
    printf("page type: %d\n", page_type);

    // Write current size
    int size = GetSize();
    out_file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    printf("size: %d\n", size);

    // Write key-child pairs
    for (int i = 0; i < size; i++)
    {
        out_file.write(reinterpret_cast<const char*>(&keys_[i]),
                       sizeof(keys_[i]));
        out_file.write(reinterpret_cast<const char*>(&child_page_ids_[i]),
                       sizeof(child_page_ids_[i]));
        printf("key: %d, child_page_id: %d\n", keys_[i], child_page_ids_[i]);
    }

    // Calculate padding to PAGE_SIZE
    int padding = PAGE_SIZE - (size * 2 * sizeof(int) +
                               8);  // 8 bytes for page type and size
    char padding_byte = 0;          // Use 0 for padding
    for (int i = 0; i < padding; i++)
    {
        out_file.write(&padding_byte, sizeof(padding_byte));
    }
    printf("padding: %d\n", padding);

    out_file.close();
}

int
BTreeInternalPage::GetChildPageId(int key) const
{
    auto it = std::lower_bound(keys_.begin(), keys_.end(), key);

    // we are looking for the key nearest to being >= the key we are looking
    // for. Then return that child page id.
    if (it != keys_.end())
    {
        size_t index = std::distance(keys_.begin(), it);
        return child_page_ids_[index];
    }

    return -1;
}