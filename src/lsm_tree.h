#ifndef LSM_TREE_H
#define LSM_TREE_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "include/storage/page/b_tree_page.h"
/**
 * The LSM Tree class
 *
 * Responsible for merging BTree files of the same level
 * into a new file with 1 greater level.
 *
 * It will merge the leaves into a leaf file and the internal nodes into an
 * internal node file. After merging is complete, the leaf file will be appended
 * to the end of the internal node file. This maintains the structure of the
 * BTree SST for a complexity of O(NlogN + N) where N is the number of pages in
 * the BTree.
 */
class LSMTree
{
   public:
    explicit LSMTree();

    std::string GetFilename() const;

    // merge 2 btree files of the same level into a new file +1 level
    void MergeBtrees(const std::string &btree1_filename,
                     const std::string &btree2_filename);

    // merge 2 pages into a new page and a potential overflow page.
    // Page 1 is considered "more recent" than Page 2, and so the keys in Page 1
    // will be prioritized. Tombstones will remove the key from the page. The
    // overflow page will contain the keys that could not fit in the new page
    // and will be used in the next merge.
    void MergePages(BTreePage *page1, BTreePage *page2, BTreePage *output_page,
                    BTreePage *overflow_page);

   private:
    int level_;  // The level of the current LSM Tree
    std::string
        filename_;  // The filename in the form of
                    // "sst_{level}_{timestamp}.sst". This way, we can extract
                    // the level from the filename instead of inside the file

    // helper to determine the new level and filename of the merged BTree
    bool TestAndSetLevelAndFIlename(const std::string &btree1_filename,
                                    const std::string &btree2_filename);

    // Read the page from the input file and deserialize into a BTreePage object
    BTreePage *ReadPage(std::ifstream &input);

    // Write the page to the output file
    void WritePage(std::ofstream &output, BTreePage *page);
};

#endif
