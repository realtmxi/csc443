#include "memtable.h"

#include "include/common/config.h"

Memtable::Memtable(size_t memtable_size)
    : max_size_(memtable_size / 8), current_size_(0)
{
    // print memtable size both in mb and number of key-value pairs
    printf("Memtable size: %lu MB\n", memtable_size / 1024 / 1024);
    printf("Max key-value pairs: %lu\n", max_size_);
}

/* Insert a KV pair into the memtable. */
void
Memtable::Put(int key, int value)
{
    t.insert(key, value);
    current_size_++;
}

/* Search for a value associated with the given key in the Memtable. */
int
Memtable::Get(int key)
{
    return t.search(key);
}

/* Get the current number of Memtable entries. */
size_t
Memtable::GetSize() const
{
    return current_size_;
}

/* Get the key-value pairs within the specified range. */
std::vector<std::pair<int, int>>
Memtable::Scan(int key1, int key2)
{
    return t.scan(key1, key2);
}

/* Check if the Memtable is full. */
bool
Memtable::IsFull() const
{
    return current_size_ >= max_size_;
}

/* Clear the Memtable. */
void
Memtable::Clear()
{
    t.clear();
    current_size_ = 0;
}
