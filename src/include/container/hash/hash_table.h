template <typename K, typename V>

class HashTable {
  public:
    HashTable();
    virtual ~HashTable();

    virtual auto Find(const K &key, V &value) -> bool = 0;
    virtual auto Insert(const K &key, const V &value) = 0;
    virtual auto Remove(const K &key) -> bool = 0;
};