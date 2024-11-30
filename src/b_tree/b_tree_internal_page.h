#pragma once

#include <string>
#include <vector>

#include "b_tree_page.h"

class BTreeInternalPage : public BTreePage
{
   public:
    explicit BTreeInternalPage(
        const std::vector<std::pair<int, int>>& key_child_pairs);

    void WriteToDisk(const std::string& filename) const;

    int GetChildPageId(int key) const;

   private:
    std::vector<int> keys_;            // Keys stored in the internal page
    std::vector<int> child_page_ids_;  // Corresponding child page IDs
};
