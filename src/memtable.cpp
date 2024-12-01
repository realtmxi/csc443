#include "memtable.h"

#include "include/common/config.h"

Memtable::Memtable() : max_size(MEMTABLE_SIZE / 4), current_size(0) {}

Memtable::Memtable(size_t memtable_size)
    : max_size(memtable_size), current_size(0)
{
}

/* Insert a KV pair into the memtable. */
void
Memtable::put(int key, int value)
{
    t.insert(key, value);
    current_size++;
}

/* Search for a value associated with the given key in the Memtable. */
int
Memtable::get(int key)
{
    return t.search(key);
}

/* Get the current number of Memtable entries. */
size_t
Memtable::getSize() const
{
    return current_size;
}

/* Get the key-value pairs within the specified range. */
std::vector<std::pair<int, int>>
Memtable::scan(int key1, int key2)
{
    return t.scan(key1, key2);
}

/* Check if the Memtable is full. */
bool
Memtable::isFull() const
{
    return current_size >= max_size;
}

/* Clear the Memtable. */
void
Memtable::clear()
{
    t.clear();
    current_size = 0;
}
