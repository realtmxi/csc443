#include <string>

#include "buffer/buffer_pool_manager.h"

#define MappingType std::pair<KeyType, ValueType>

#define INDEX_TEMPLATE_ARGUMENTS template <typename KeyType, typename ValueType, typename KeyComparator>

// define page type enum
enum class IndexPageType
{
  INVALID_INDEX_PAGE = 0,
  LEAF_PAGE = 1,
  INTERNAL_PAGE = 2,
}

/** 
 * This is a base class that the internal page and leaf page inherit from.
 * It is a header part for each B tree page and contains information shared by 
 * both leaf page and internal page.
 *
 * Header format (size in byte, 12 bytes in total):
 * -------------------------------------------------------
 * | PageType (4) | CurrentSize (4) | MaxSize (4) | ...  |
 * -------------------------------------------------------
 */

class BTreePage {
 public:
  BTreePage() = delete;
  BTreePage(const BTreePage &other) = delete;
  ~BTreePage() = delete;

  auto IsLeafPage() const -> bool;
  void SetPageType(IndexPageType page_type);

  auto GetSize() const -> int;
  void SetSize(int size);
  void ChangeSizeBy(int amount);

  auto GetMaxSize() const -> int;
  void SetMaxSize(int max_size);
  auto GetMinSize() const -> int;

 private:
  // Member variables, attributes that both internal and leaf page share
  IndexPageType page_type_ __attribute__((__unused__));
  // Number of key & value pairs in a page
  int size_ __attribute__((__unused__));
  // Max number of key & value pairs in a page
  int max_size_ __attribute__((__unused__));
}