

#include "b_tree_manager.h"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>  // Add this line
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/common/config.h"
#include "b_tree_page.h"

BTreeManager::BTreeManager(const std::string &filename) : filename_(filename) {}

int
BTreeManager::Get(int key)
{
    BTreePage page = TraverseToKey(key);
    if (page.IsLeafPage())
    {
        return page.Get(key);
    }
    return -1;
}

std::vector<std::pair<int, int>>
BTreeManager::Scan(int start_key, int end_key)
{
    return TraverseRange(start_key, end_key);
}

std::string
BTreeManager::Merge(const std::string &filename_to_merge)
{
    return MergeBTreeFromFile(filename_to_merge);
}

BTreePage
BTreeManager::ReadPageFromDisk(int page_id, const std::string &filename) const
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open B-tree file: " + filename);
    }

    // check if page_id * PAGE_SIZE is within the file size
    file.seekg(0, std::ios::end);
    int file_size = file.tellg();
    if (page_id * PAGE_SIZE >= file_size)
    {
        return BTreePage();
    }

    // Use a byte buffer (char did not work for me)
    std::byte buffer[PAGE_SIZE];
    file.seekg(page_id * PAGE_SIZE);
    if (!file.good())
    {
        return BTreePage();
    }

    file.read(reinterpret_cast<char *>(buffer), PAGE_SIZE);
    file.close();

    std::byte *buffer_ptr = buffer;

    // Read in the page type (4 bytes)
    BTreePageType page_type =
        *reinterpret_cast<const BTreePageType *>(buffer_ptr);
    buffer_ptr += sizeof(BTreePageType);
    if (page_type == BTreePageType::INVALID_PAGE)
    {
        return BTreePage();
    }

    // Read in the size of the page (4 bytes)
    int size = *reinterpret_cast<const int *>(buffer_ptr);
    buffer_ptr += sizeof(int);
    if (size == 0)
    {
        return BTreePage();
    }

    // Read in the key-value pairs (8 bytes each pair)
    int num_pairs = size;
    std::vector<std::pair<int, int>> pairs;
    for (int i = 0; i < num_pairs; i++)
    {
        int key = *reinterpret_cast<const int *>(buffer_ptr);
        buffer_ptr += sizeof(int);
        int value = *reinterpret_cast<const int *>(buffer_ptr);
        buffer_ptr += sizeof(int);
        pairs.push_back({key, value});
    }

    if (pairs.empty())
    {
        return BTreePage();
    }

    // Create a new BTreePage object
    BTreePage page(pairs);
    page.SetPageType(page_type);
    page.SetSize(size);
    page.SetPageId(page_id);

    // if the page is an internal page, read the child page IDs
    if (page_type == BTreePageType::INTERNAL_PAGE)
    {
        int child_page_id = *reinterpret_cast<const int *>(buffer_ptr);
        page.SetValueAtIdx(-1, child_page_id);
    }

    return page;
}

BTreePage
BTreeManager::TraverseToKey(int key) const
{
    // Start at the root page
    BTreePage page = ReadPageFromDisk(0, filename_);
    while (!page.IsLeafPage())
    {
        // Fine the child of the root that leads us to the key and read it
        int child_page_id = page.FindChildPage(key);
        page = ReadPageFromDisk(child_page_id, filename_);

        // If the page is invalid, return an empty page
        if (page.GetPageType() == BTreePageType::INVALID_PAGE)
        {
            return BTreePage();
        }
    }

    return page;
}

std::vector<std::pair<int, int>>
BTreeManager::TraverseRange(int start_key, int end_key) const
{
    std::vector<std::pair<int, int>> result;

    // Start at the root page
    BTreePage page = ReadPageFromDisk(0, filename_);
    while (!page.IsLeafPage())
    {
        // Find the child of the root that leads us to the start key and read it
        int child_page_id = page.FindChildPage(start_key);
        page = ReadPageFromDisk(child_page_id, filename_);

        // If the page is invalid, return an empty result
        if (page.GetPageType() == BTreePageType::INVALID_PAGE)
        {
            return result;
        }
    }

    // page is a leaf that contains the start key. Scan the range
    while (page.IsLeafPage())
    {
        std::vector<std::pair<int, int>> pairs = page.Scan(start_key, end_key);
        result.insert(result.end(), pairs.begin(), pairs.end());

        // The leaf pages are stored consecutively on disk, so if we have not
        // reached a key greater than end_key, we can read the next page
        if (page.GetMaxKey() >= end_key)
        {
            break;
        }

        int next_page_id = page.GetPageId() + 1;
        page = ReadPageFromDisk(next_page_id, filename_);
    }

    return result;
}
std::string
BTreeManager::MergeBTreeFromFile(const std::string &filename_to_merge)
{
    // Determine the filename for the merged B-tree
    std::string merge_filename =
        DetermineMergeFilename(filename_, filename_to_merge);

    std::string temp_leaf_filename = "temp_leaf_file.sst";
    std::string temp_internal_filename = "temp_internal_file.sst";

    // Step 1: Open two input buffers for the BTree files
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open B-tree file: " << filename_ << std::endl;
        if (file.fail())
        {
            std::cerr << "File open failed due to: " << strerror(errno)
                      << std::endl;
        }
        throw std::runtime_error("Failed to open B-tree file: " + filename_);
    }

    std::ifstream file_to_merge(filename_to_merge, std::ios::binary);
    if (!file_to_merge.is_open())
    {
        throw std::runtime_error("Failed to open B-tree file: " +
                                 filename_to_merge);
    }

    printf("Merging %s and %s into %s\n", filename_.c_str(),
           filename_to_merge.c_str(), merge_filename.c_str());

    // Step 2: Find the leaf pages of each BTree to start the merge
    int page_id = 0;
    int page_id_to_merge = 0;
    BTreePage page = ReadPageFromDisk(page_id, filename_);
    BTreePage page_to_merge =
        ReadPageFromDisk(page_id_to_merge, filename_to_merge);

    printf("Read root pages\n");

    while (!page.IsLeafPage())
    {
        page_id = page.FindChildPage(INT_MIN);
        page = ReadPageFromDisk(page_id, filename_);
        printf("pageid: %d\n", page_id);
    }

    printf("Found leaf page for %s\n", filename_.c_str());

    while (!page_to_merge.IsLeafPage())
    {
        page_id_to_merge = page_to_merge.FindChildPage(INT_MIN);
        page_to_merge = ReadPageFromDisk(page_id_to_merge, filename_to_merge);
        printf("pageid: %d\n", page_id_to_merge);
    }

    printf("Found leaf page for %s\n", filename_to_merge.c_str());

    std::vector<std::pair<int, int>> merged_pairs;
    std::vector<int> internal_node_max_keys;

    auto pairs = page.GetKeyValues();
    auto pairs_to_merge = page_to_merge.GetKeyValues();
    auto it1 = pairs.begin();
    auto it2 = pairs_to_merge.begin();

    printf("Starting merge\n");

    // Step 3: Merge the leaf pages into a new BTreePage
    while (page.IsLeafPage() || page_to_merge.IsLeafPage())
    {
        while (it1 != pairs.end() || it2 != pairs_to_merge.end())
        {
            // If both havent ended, compare with each other
            if (it1 != pairs.end() && it2 != pairs_to_merge.end())
            {
                if (it1->first < it2->first)
                {
                    merged_pairs.push_back(*it1);
                    ++it1;
                }
                else if (it1->first > it2->first)
                {
                    merged_pairs.push_back(*it2);
                    ++it2;
                }
                else
                {
                    // If the keys are the same, choose the value from the
                    // newer page
                    merged_pairs.push_back(*it1);
                    ++it1;
                    ++it2;
                }
            }
            // If one has ended, add the other
            else if (it1 != pairs.end())
            {
                merged_pairs.push_back(*it1);
                ++it1;
            }
            else if (it2 != pairs_to_merge.end())
            {
                merged_pairs.push_back(*it2);
                ++it2;
            }

            // Once we hit the max size, write the page to disk
            if (merged_pairs.size() == MEMTABLE_SIZE)
            {
                printf("BTreePage Full, writing to disk\n");
                printf("max key %d\n", merged_pairs.back().first);
                WriteLeafPage(temp_leaf_filename, merged_pairs,
                              internal_node_max_keys);
                merged_pairs.clear();
            }
        }

        // Handle when an iterator reaches the end
        if (it1 == pairs.end() && page.IsLeafPage())
        {
            page_id += 1;
            printf("iterator 1 at end, reading next page with id: %d\n",
                   page_id);
            page = ReadPageFromDisk(page_id, filename_);

            // Check if the page is valid and still a leaf
            if (page.IsLeafPage())
            {
                pairs = page.GetKeyValues();
                it1 = pairs.begin();
            }
        }

        if (it2 == pairs_to_merge.end() && page_to_merge.IsLeafPage())
        {
            page_id_to_merge += 1;
            printf("iterator 2 at end, reading next page with id: %d\n",
                   page_id_to_merge);
            page_to_merge =
                ReadPageFromDisk(page_id_to_merge, filename_to_merge);

            // Check if the page is valid and still a leaf
            if (page_to_merge.IsLeafPage())
            {
                pairs_to_merge = page_to_merge.GetKeyValues();
                it2 = pairs_to_merge.begin();
                printf("Read next page, is leaf: %d\n",
                       static_cast<int>(page_to_merge.IsLeafPage()));
                // print the size of page_to_merge
                printf("page_to_merge size: %ld\n", pairs_to_merge.size());
            }
        }

        // If both iterators have reached the end and there are no more leaf
        // pages to read, break the loop
        if (it1 == pairs.end() && it2 == pairs_to_merge.end())
        {
            printf("Both iterators at end\n");

            break;
        }
    }

    // After the loop, write any remaining pairs to disk
    if (!merged_pairs.empty())
    {
        // there can only be max MEMTABLE_SIZE * 2 keys left. SO we can just
        // write either 1 or 2 pages, depending on the size of the merged_pairs
        std::vector<std::pair<int, int>> pairs1;
        std::vector<std::pair<int, int>> pairs2;
        for (size_t i = 0; i < merged_pairs.size(); i++)
        {
            if (i < MEMTABLE_SIZE)
            {
                pairs1.push_back(merged_pairs[i]);
            }
            else
            {
                pairs2.push_back(merged_pairs[i]);
            }
        }

        printf(
            "Writing remaining pairs, size1: %ld, size2: %ld\n, memtable_size: "
            "%d\n",
            pairs1.size(), pairs2.size(), MEMTABLE_SIZE);
        // Write the first page
        WriteLeafPage(temp_leaf_filename, pairs1, internal_node_max_keys);

        // Write the second page if it exists
        if (!pairs2.empty())
        {
            WriteLeafPage(temp_leaf_filename, pairs2, internal_node_max_keys);
        }
    }

    // Using the max keys from the internal nodes, construct the internal nodes
    ConstructInternalNodes(temp_internal_filename, internal_node_max_keys);

    // combine the internal and leaf pages into a single file
    std::ifstream temp_leaf_file(temp_leaf_filename, std::ios::binary);
    std::ifstream temp_internal_file(temp_internal_filename, std::ios::binary);

    // print the sizes of each file
    // Open the output file in binary mode
    std::ofstream output_file(merge_filename, std::ios::binary);

    // Check if the file is opened properly
    if (!output_file.is_open())
    {
        throw std::runtime_error("Failed to open output file: " +
                                 merge_filename);
    }

    // Print sizes of the input files
    temp_leaf_file.seekg(0, std::ios::end);
    printf("Leaf file size: %ld\n", static_cast<long>(temp_leaf_file.tellg()));
    temp_internal_file.seekg(0, std::ios::end);
    std::streampos internal_size = temp_internal_file.tellg();
    printf("Internal file size: %ld\n", static_cast<long>(internal_size));

    // Copy contents of internal and leaf files to the output file
    temp_internal_file.seekg(
        0, std::ios::beg);  // Move to the start of the internal file
    output_file << temp_internal_file.rdbuf();  // Copy internal nodes

    temp_leaf_file.seekg(0,
                         std::ios::beg);  // Move to the start of the leaf file
    output_file << temp_leaf_file.rdbuf();  // Copy leaf nodes

    // Now that data has been written, check the size of the output file
    output_file.seekp(0, std::ios::end);  // Move the pointer to the end
    std::streampos output_size = output_file.tellp();
    printf("Output file size: %ld\n", static_cast<long>(output_size));

    // Close the files and remove the temporary files
    file.close();
    file_to_merge.close();
    output_file.close();
    temp_leaf_file.close();
    temp_internal_file.close();
    std::remove(temp_leaf_filename.c_str());
    std::remove(temp_internal_filename.c_str());

    return merge_filename;
}

void
BTreeManager::WriteLeafPage(std::string &filename,
                            std::vector<std::pair<int, int>> &keys,
                            std::vector<int> &internal_node_max_keys)
{
    BTreePage page(keys);
    page.SetPageType(BTreePageType::LEAF_PAGE);
    page.SetSize(keys.size());
    printf("Writing leaf page to disk\n");
    page.WriteToDisk(filename);

    // Add the max key to the internal node
    internal_node_max_keys.push_back(page.GetMaxKey());
}

void
BTreeManager::ConstructInternalNodes(std::string &filename,
                                     std::vector<int> &max_keys)
{
    // Holds all layers of internal nodes
    std::vector<std::vector<BTreePage>> internal_page_layers;

    printf("max keys size: %ld\n", max_keys.size());
    // Loop until only one node remains in the current layer
    while (!max_keys.empty())
    {
        printf("max keys size: %ld\n", max_keys.size());
        std::vector<int> internal_node_max_keys;
        std::vector<BTreePage> internal_pages;

        // Construct internal nodes for the current layer
        for (size_t i = 0; i < max_keys.size(); i += MEMTABLE_SIZE)
        {
            std::vector<std::pair<int, int>> keys;

            for (size_t j = i; j < i + MEMTABLE_SIZE && j < max_keys.size();
                 j++)
            {
                keys.push_back(
                    {max_keys[j], -1});  // -1 is a placeholder for now
            }

            // Create and populate the internal node
            BTreePage page(keys);
            page.SetPageType(BTreePageType::INTERNAL_PAGE);
            page.SetSize(keys.size());
            internal_pages.push_back(page);

            // Record the max key for the next layer
            internal_node_max_keys.push_back(page.GetMaxKey());
        }

        // Save the current layer and prepare for the next
        internal_page_layers.push_back(internal_pages);
        max_keys = internal_node_max_keys;
        if (max_keys.size() == 1)
        {
            break;
        }
    }

    // loop through the nodes and set the child_page_ids
    // The root is 0 so the roots first child is 1, the second child is 2, etc.
    // The next layer starts at the next available page_id

    // Start at the root
    printf("Writing internal nodes to disk\n");
    int page_id = 0;
    for (auto internal_pages : internal_page_layers)
    {
        std::vector<BTreePage> next_internal_pages;

        for (auto page : internal_pages)
        {
            printf("page_id: %d\n", page_id);
            page.SetPageId(page_id);

            // Set the child page ids for the internal node
            for (size_t k = 0; k < static_cast<size_t>(page.GetSize()); k++)
            {
                page_id += 1;
                page.SetValueAtIdx(k, page_id);
            }
            // as an internal node, the last child is the right most child
            page.SetValueAtIdx(page.GetSize(), page_id + page.GetSize() + 1);
            page_id += 1;

            // Write the internal node to disk
            page.WriteToDisk(filename);
            printf("Wrote to disk\n");

            // Save the next layer of internal nodes
            next_internal_pages.push_back(page);
        }

        // Prepare for the next layer
        internal_pages = next_internal_pages;
    }
}

std::string
BTreeManager::DetermineMergeFilename(const std::string &filename1,
                                     const std::string &filename2) const
{
    // Check if they each have the same level. if not, return an error
    std::string level1 = filename1.substr(filename1.find("sst_") + 4, 4);
    std::string level2 = filename2.substr(filename2.find("sst_") + 4, 4);
    if (level1 != level2)
    {
        throw std::runtime_error("Cannot merge B-trees of different levels");
    }

    // increment the level.
    int new_level = std::stoi(level1) + 1;

    // Create a new timestamp
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                      now.time_since_epoch())
                      .count();

    // The filename is the format sst_level_timestamp.sst
    std::stringstream new_filename;
    new_filename << "sst_" << std::setfill('0') << std::setw(4) << new_level
                 << "_" << (now_ms) << ".sst";

    printf("Merging into %s\n", new_filename.str().c_str());

    return new_filename.str();
}