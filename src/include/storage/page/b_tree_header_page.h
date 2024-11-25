#include "common/config.h"

/** 
 * The header page used to retrieve the root page, preventing potential race 
 * condition. 
 */
class BTreeHeaderPage {
 public:
  BTreeHeaderPage() = delete;
  BTreeHeaderPage(const BTreeHeaderPage &other) = delete;

  page_id_t root_page_id_;
};