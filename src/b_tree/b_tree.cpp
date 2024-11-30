#include "b_tree.h"

#include <cstdio>

#include "b_tree_internal_page.h"
#include "b_tree_leaf_page.h"

BTree::BTree(const std::vector<std::pair<int, int>>& data)
{
    // Create a single leaf page with the data.
    BTreeLeafPage leaf_page(data);

    // Create a single internal page with the key being the max key in the leaf
    // page. The page ID is set to 1 (assuming a single-page context here).
    BTreeInternalPage internal_page({{leaf_page.GetMaxKey(), 1}});

    // Store the pages in the tree.
    leaf_pages_.push_back(leaf_page);
    internal_pages_.push_back(internal_page);
}

void
BTree::SaveBTreeToDisk(const std::string& filename)
{
    // Save each internal page to disk.
    for (const auto& internal_page : internal_pages_)
    {
        printf("Writing internal page to disk\n");
        internal_page.WriteToDisk(filename);
    }

    // Save each leaf page to disk.
    for (const auto& leaf_page : leaf_pages_)
    {
        printf("Writing leaf page to disk\n");
        leaf_page.WriteToDisk(filename);
    }
}
