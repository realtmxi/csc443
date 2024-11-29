#include <string>
#include <cstdint>
#include <vector>
#include <memory>

#include "buffer/buffer_pool_manager.h"
#include "page.h"

#define MappingType std::pair<KeyType, ValueType>

#define INDEX_TEMPLATE_ARGUMENTS template <typename KeyType, typename ValueType, typename KeyComparator>

// define page type enum
enum class BTreePageType {
  INVALID_PAGE = 0,
  INTERNAL_PAGE = 1,
  LEAF_PAGE = 2,
};

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

class BTreePage : public Page {
 public:
  BTreePage() = delete;
  ~BTreePage() = default;

  auto IsLeafPage() const -> bool;
  auto IsRootPage() const -> bool;

  void SetPageType(BTreePageType page_type);
  auto GetPageType() const -> BTreePageType;

  auto GetSize() const -> int;
  void SetSize(int size);
  void IncreaseSize(int amount);
  void DecreaseSize(int amount);

  auto GetMaxSize() const -> int;
  void SetMaxSize(int max_size);

  auto GetParentPageId() const -> int;
  void SetParentPageId(int parent_page_id);

 private:
  BTreePageType page_type_;
  int size_;
  int max_size_;
  int parent_page_id_;
};
