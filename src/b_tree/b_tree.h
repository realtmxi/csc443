#ifndef B_TREE_H
#define B_TREE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "b_tree_internal_page.h"
#include "b_tree_leaf_page.h"

/**
 * The BTree class represents the overall structure of a B-Tree.
 * It contains a list of internal and leaf pages.
 */
class BTree
{
   public:
    // Constructor that initializes the tree with key-value pairs.
    explicit BTree(const std::vector<std::pair<int, int>>& data);

    // Saves the BTree structure to disk as a binary file.
    void SaveBTreeToDisk(const std::string& filename);

   private:
    // List of internal pages.
    std::vector<BTreeInternalPage> internal_pages_;

    // List of leaf pages.
    std::vector<BTreeLeafPage> leaf_pages_;
};

#endif  // B_TREE_H
