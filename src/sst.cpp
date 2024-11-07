#include "sst.h"
#include <sys/stat.h>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>

SSTable::SSTable(const std::string &path) : file_path(path) {
  struct stat buf;
  if (stat(file_path.c_str(), &buf) != 0) {
    throw std::runtime_error("Failed to stat SSTable file");
  } else {
    // Dividing the file size by the size of one kv pair.
    num_entries = buf.st_size / (sizeof(int) * 2);
  }
}

int
SSTable::readKey(int fd, off_t offset)
{
  int key;
  pread(fd, &key, sizeof(int), offset);
  return key;
}

int
SSTable::readValue(int fd, off_t offset)
{
  int value;
  pread(fd, &value, sizeof(int), offset);
  return value;
}

/* Use a binary search to find the start of the range and continue with a scan
  until encounter a key key outside of the range. */
std::vector<std::pair<int, int>>
SSTable::scan(int key1, int key2)
{
  std::vector<std::pair<int, int>> result;

  int fd = open(file_path.c_str(), O_RDONLY);

  // Binary search to find the start of the range.
  size_t left = 0;
  size_t right = num_entries - 1;
  size_t start_pos = num_entries;
  
  while (left <= right)
  {
    size_t mid = (left + right) / 2;
    off_t ofs = mid * sizeof(int) * 2;
    int key = readKey(fd, ofs);

    if (key < key1)
    {
      left = mid + 1;
    }
    else
    {
      right = mid - 1;
      start_pos = mid;
    }
  }

  // Sequentially read from start position
  for (size_t i = start_pos; i < num_entries; i++)
  {
    off_t ofs = i * sizeof(int) * 2;
    int key = readKey(fd, ofs);
    if (key > key2)
      break;
    int value = readValue(fd, ofs + sizeof(int));
    result.push_back({key, value});
  }

  close(fd);

  return result;
}

/* Perform a binary search for an exact key match in SSTable. */
int
SSTable::get(int key)
{
  int fd = open(file_path.c_str(), O_RDONLY);
  if (fd < 0)
    return -1;

  size_t left = 0;
  size_t right = num_entries - 1;

  while (left <= right)
  {
    size_t mid = left + (right - left) / 2;
    off_t ofs = mid * sizeof(int) * 2;
    int mid_key = readKey(fd, ofs);

    if (mid_key == key)
    {
      int value = readValue(fd, ofs + sizeof(int));
      close(fd);
      return value;
    }
    else if (mid_key < key)
    {
      left = mid + 1;
    }
    else
    {
      right = mid - 1;
    }
  }

  close (fd);
  return -1; // Key not found
}

/* Write key-value pairs to an SST file in binary format. */