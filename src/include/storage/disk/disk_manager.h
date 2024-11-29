#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <fstream>
#include <mutex>
#include <string>

#include "common/config.h"

/**
 * DiskManager takes care of the allocation and deallocation of pages within a 
 * database. It performs the reading and writing of pages to and from disk.
 */
class DiskManager {
 public:
  /**
   * Initialize the disk manager with a database file.
   * @param db_file the database file name.
   */
  explicit DiskManager(const std::string &db_file);

  /** Shut down the disk manager and close resources. */
  ~DiskManager();

  /**
   * Read a page from disk.
   * @param page_id the ID of the page to read.
   * @param[out] page_data the buffer to store the read data.
   */
  void ReadPage(page_id_t page_id, char *page_data);

  /**
   * Write a page to disk.
   * @param page_id the ID of the page to write.
   * @param page_data the buffer containing the page data.
   */
  void WritePage(page_id_t page_id, const char *page_data);

  /**
   * Delete a page from disk.
   * @param page_id the ID of the page to delete.
   */
  void DeletePage(page_id_t page_id);

 private:
  std::fstream db_io_;               // File stream for database I/O
  std::string file_name_;            // Name of the database file
  std::mutex db_io_latch_;           // Mutex for thread-safe file access
};

#endif  // DISK_MANAGER_H
