#ifndef B_TREE_PAGE_H
#define B_TREE_PAGE_H

#include <cstdint>
#include <string>

// Enum representing the type of a B-Tree page.
enum class BTreePageType
{
    INVALID_PAGE = 0,
    INTERNAL_PAGE = 1,
    LEAF_PAGE = 2,
};

/**
 * Base class for all B-Tree pages.
 * Includes common properties and methods.
 */
class BTreePage
{
   public:
    // Constructor and destructor.
    BTreePage();
    virtual ~BTreePage();

    // Determine if the page is a leaf page.
    bool IsLeafPage() const;
    bool IsInternalPage() const;

    // Set and get the page type.
    void SetPageType(BTreePageType page_type);
    BTreePageType GetPageType() const;

    // Set and get the size of the page.
    void SetSize(int size);
    int GetSize() const;

    // Set and get the page ID.
    void SetPageId(int page_id);
    int GetPageId() const;

    int Get(int key) const;
    std::vector<std::pair<int, int>> Scan(int key1, int key2) const;

    void WriteToDisk(const std::string& filename) const;

   private:
    BTreePageType page_type_ = BTreePageType::INVALID_PAGE;  // Page type.
    int size_ = 0;      // Number of elements.
    int page_id_ = -1;  // Page ID (optional).
    std::vector<int> keys_;    // List of keys.
    std::vector<int> values_;  // Corresponding values / child page IDs.
};

#endif  // B_TREE_PAGE_H
