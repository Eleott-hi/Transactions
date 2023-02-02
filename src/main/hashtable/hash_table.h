#ifndef A6_SRC_MAIN_HASHTABLE_HASH_TABLE_H_
#define A6_SRC_MAIN_HASHTABLE_HASH_TABLE_H_

#include <list>
#include <map>

#include "async_pool.h"
#include "key_value_storage.h"

namespace s21 {

class HashTable : public KeyValueStorage {
 public:
  explicit HashTable(size_t capacity);
  ~HashTable() = default;
  HashTable(const HashTable&) = delete;
  HashTable(HashTable&&) = delete;
  void operator=(const HashTable&) = delete;
  void operator=(HashTable&&) = delete;

  bool Set(const K& key, const V& value, int lifetime = -1) override;
  [[nodiscard]] V Get(const K& key) const override;
  [[nodiscard]] bool Exists(const K& key) const override;
  bool Delete(const K& key) override;
  bool Update(const K& key, const V& value) override;
  [[nodiscard]] std::vector<K> Keys() const override;
  bool Rename(const K& from, const K& to) override;
  [[nodiscard]] int Ttl(const K& key) const override;
  [[nodiscard]] std::vector<K> Find(const V& value) const override;
  [[nodiscard]] std::vector<V> ShowAll() const override;
  int Upload(const std::string& filename) override;
  int Export(const std::string& filename) const override;

 private:
  struct Node {
    K key;
    V value;
    int lifetime;

    bool operator==(const Node& other) { return key == other.key; }
  };

  size_t capacity_ = 0;
  std::vector<std::list<Node>> data_;
  std::map<K, size_t> deletion_queue_;
  AsyncPool pool_;
  mutable std::recursive_mutex mtx_;

  size_t CalcHashCode(const K& key) const;
  size_t CalcIndex(const K& key) const;
};

}  // namespace s21

#endif  // A6_SRC_MAIN_HASHTABLE_HASH_TABLE_H_
