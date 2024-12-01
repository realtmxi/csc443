#ifndef B_TREE_MANAGER_H
#define B_TREE_MANAGER_H

#include <string>
#include <utility>
#include <vector>

#include "b_tree_page.h"

class BTreeManager
{
   public:
    explicit BTreeManager(const std::string& filename);
    int Get(int key);
    std::vector<std::pair<int, int>> Scan(int start_key, int end_key);

    // TODO: Merge two BTrees and increase the level of the LSM
    void Merge(const std::string& filename_to_merge);

   private:
    std::string filename_;
    BTreePage ReadPageFromDisk(int page_id) const;
    BTreePage TraverseToKey(int key) const;
    std::vector<std::pair<int, int>> TraverseRange(int start_key,
                                                   int end_key) const;
    void MergeBTreeFromFile(const std::string& filename_to_merge);
};

#endif
