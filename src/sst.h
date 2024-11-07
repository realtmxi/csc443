// Copyright (C) Microsoft Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SST_H
#define SST_H

#include <string>
#include <vector>
#include <utility>


/** Sorted String Table (SSTable) class.
 * 
 */
class SSTable {
  private:
    std::string file_path;
    size_t num_entries;
    
    int readKey(int fd, off_t offset);
    int readValue(int fd, off_t offset);
    
  public:
    SSTable(const std::string &path);

    std::vector<std::pair<int, int>> scan(int key1, int key2);
    int get(int key);

    void write(const std::string& filename, 
               const std::vector<std::pair<int, int>>& data);
}

#endif