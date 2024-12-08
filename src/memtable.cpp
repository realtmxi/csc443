#include "memtable.h"

#include "config.h"

Memtable::Memtable(int memtable_size) : max_size_(memtable_size / 8) {}

/* Insert a KV pair into the memtable. */
void
Memtable::Put(int key, int value)
{
    t.insert(key, value);
}

/* Search for a value associated with the given key in the Memtable. */
int
Memtable::Get(int key)
{
    return t.search(key);
}

/* Delete a KV pair from the Memtable. */
void
Memtable::Delete(int key)
{
    t.insert(key, INT_MAX);
}

/* Get the current number of Memtable entries. */
int
Memtable::GetSize()
{
    return t.GetSize();
}

/* Get the key-value pairs within the specified range. */
std::vector<std::pair<int, int>>
Memtable::Scan(int key1, int key2)
{
    return t.scan(key1, key2);
}

/* Check if the Memtable is full. */
bool
Memtable::IsFull()
{
    return t.GetSize() >= max_size_;
}

/* Clear the Memtable. */
void
Memtable::Clear()
{
    t.clear();
}
