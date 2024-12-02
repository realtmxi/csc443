#ifndef B_TREE_H
#define B_TREE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "b_tree_page.h"

class BTree
{
   public:
    // Constructor that transforms a list of key-value pairs into a BTree.
    explicit BTree(const std::vector<std::pair<int, int>>& data);

    // Save the BTree to disk for a given filename.
    void SaveBTreeToDisk(const std::string& filename);

   private:
    std::vector<BTreePage> internal_pages_;
    std::vector<BTreePage> leaf_pages_;

    void ConstructLeafPages(const std::vector<std::pair<int, int>>& data,
                            std::vector<int>& max_keys);

    void ConstructInternalNodes(std::vector<int>& max_keys);
};

#endif
