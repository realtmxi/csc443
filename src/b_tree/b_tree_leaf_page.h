#ifndef B_TREE_LEAF_PAGE_H
#define B_TREE_LEAF_PAGE_H

#include <string>
#include <utility>
#include <vector>

#include "b_tree_page.h"

/**
 * Leaf page for B-Tree.
 * Stores keys and their associated values.
 */
class BTreeLeafPage : public BTreePage
{
   public:
    // Constructor that initializes the leaf page with key-value pairs.
    explicit BTreeLeafPage(const std::vector<std::pair<int, int>>& kv_pairs);

    // Writes the leaf page to disk.
    void WriteToDisk(const std::string& filename) const;

    // Get the value associated with the given key.
    int Get(int key) const;

    int GetMaxKey() const;

    // Get the key-value pairs within the specified range.
    std::vector<std::pair<int, int>> Scan(int key1, int key2) const;

   private:
    std::vector<int> keys_;    // List of keys.
    std::vector<int> values_;  // Corresponding values.
};

#endif