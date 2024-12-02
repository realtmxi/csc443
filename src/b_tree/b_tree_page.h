#ifndef B_TREE_PAGE_H
#define B_TREE_PAGE_H

#include <cstdint>
#include <string>
#include <vector>

enum class BTreePageType
{
    INVALID_PAGE = 0,
    INTERNAL_PAGE = 1,
    LEAF_PAGE = 2,
};

class BTreePage
{
   public:
    BTreePage();
    explicit BTreePage(const std::vector<std::pair<int, int>>& key_value_pairs);

    bool IsLeafPage() const;
    bool IsInternalPage() const;

    void SetPageType(BTreePageType page_type);
    BTreePageType GetPageType() const;

    void SetSize(int size);
    int GetSize() const;

    void SetPageId(int page_id);
    int GetPageId() const;

    // Used for the Leaf Pages
    int Get(int key) const;
    std::vector<std::pair<int, int>> Scan(int key1, int key2) const;
    // Used for the Internal Pages
    int FindChildPage(int key) const;

    int GetMaxKey() const;
    void WriteToDisk(const std::string& filename) const;

    std::vector<std::pair<int, int>> GetKeyValues() const;

    void SetValueAtIdx(int idx, int value);

   private:
    BTreePageType page_type_;
    int size_;
    int page_id_;
    std::vector<int> keys_;
    std::vector<int> values_;  // values / child page IDs.
};

#endif
