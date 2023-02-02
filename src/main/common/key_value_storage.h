#ifndef A6_SRC_MAIN_COMMON_KEY_VALUE_STORAGE_H_
#define A6_SRC_MAIN_COMMON_KEY_VALUE_STORAGE_H_

#include <vector>

#include "person.h"

namespace s21 {

class KeyValueStorage {
 public:
  using K = std::string;
  using V = Person;

  virtual ~KeyValueStorage() = default;

  virtual bool Set(const K& key, const V& value, int lifetime = -1) = 0;
  virtual V Get(const K& key) const = 0;
  virtual bool Exists(const K& key) const = 0;
  virtual bool Delete(const K& key) = 0;
  virtual bool Update(const K& key, const V& value) = 0;
  virtual std::vector<K> Keys() const = 0;
  virtual bool Rename(const K& from, const K& to) = 0;
  virtual int Ttl(const K& key) const = 0;
  virtual std::vector<K> Find(const V& value) const = 0;
  virtual std::vector<V> ShowAll() const = 0;
  virtual int Upload(const std::string& filename) = 0;
  virtual int Export(const std::string& filename) const = 0;
};

}  // namespace s21

#endif  // A6_SRC_MAIN_COMMON_KEY_VALUE_STORAGE_H_
