#ifndef B_TREE_MANAGER_H
#define B_TREE_MANAGER_H

#include <string>
#include <utility>
#include <vector>

#include "b_tree_page.h"

class BTreeManager
{
   public:
    // Constructor that initializes the BTreeManager with a specific B-tree
    // file.
    explicit BTreeManager(const std::string& filename);

    // Get a value by key from the B-tree.
    // Returns a pair with key-value if found, or an empty pair if not.
    int Get(int key);

    // Scan a range of keys in the B-tree.
    // Returns a vector of pairs representing key-value pairs within the range.
    std::vector<std::pair<int, int>> Scan(int start_key, int end_key);

    // Merge another B-tree file with the current B-tree file.
    // Combines the data from the other file and merges it with the current
    // file.
    void Merge(const std::string& filename_to_merge);

   private:
    std::string filename_;  // The current B-tree file managed by this instance.

    // Helper function to read a page from disk into a BTreePage object.
    BTreePage ReadPageFromDisk(int page_id) const;

    // Helper function to traverse the B-tree structure to find a key.
    BTreePage TraverseToKey(int key) const;

    // Helper function to scan a range of keys by traversing the B-tree.
    std::vector<std::pair<int, int>> TraverseRange(int start_key,
                                                   int end_key) const;

    // Helper function to merge two B-trees from disk into one.
    void MergeBTreeFromFile(const std::string& filename_to_merge);
};

#endif  // B_TREE_MANAGER_H
