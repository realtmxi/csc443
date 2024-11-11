#include "murmur3/MurmurHash3.h"

template <typename KeyType>
class HashFunction {
  public:
    /**
     * @param key the key to be hashed
     * @return the hash value of the key
     */
    virtual auto Gethash(KeyType key) const -> uint64_t {
      uint64_t hash[2];
      murmur3::MurmurHash3_x64_128(reinterpret_cast<const void *>(&key), 
                                    static_cast<int>(sizeof(KeyType)), 0, 
                                    reinterpret_cast<void *>(&hash));
    }
};