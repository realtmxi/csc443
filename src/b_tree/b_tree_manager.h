#ifndef B_TREE_MANAGER_H
#define B_TREE_MANAGER_H

#include <string>
#include <utility>
#include <vector>

#include "b_tree_page.h"

class BTreeManager
{
   public:
    explicit BTreeManager(const std::string& filename, int largest_lsm_level);
    int Get(int key);
    int BinarySearchGet(int key) const;
    std::vector<std::pair<int, int>> Scan(int start_key, int end_key);
    // Merge the BTree with another BTree file, return output filename
    std::string Merge(const std::string& filename_to_merge);

    // used for testing, would otherwise be private
    BTreePage TraverseToKey(int key) const;

   private:
    std::string filename_;
    int largest_lsm_level_;
    bool remove_tombstones_;
    BTreePage ReadPageFromDisk(int page_id, const std::string& filename) const;

    std::vector<std::pair<int, int>> TraverseRange(int start_key,
                                                   int end_key) const;
    std::string MergeBTreeFromFile(const std::string& filename_to_merge);
    std::string DetermineMergeFilename(const std::string& filename1,
                                       const std::string& filename2);
    void ConstructInternalNodes(std::string& filename,
                                std::vector<int>& max_keys);
    void WriteLeafPage(std::string& filename,
                       std::vector<std::pair<int, int>>& keys,
                       std::vector<int>& internal_node_max_keys);
};

#endif
