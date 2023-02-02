#include "hash_table.h"

#include <cmath>
#include <fstream>

namespace s21 {

HashTable::HashTable(size_t capacity) : capacity_(capacity), data_(capacity) {}

bool HashTable::Set(const K& key, const V& value, int lifetime) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  if (Exists(key)) return false;

  if (lifetime > -1) {
    size_t id = pool_.DelayTask(std::chrono::seconds(lifetime),
                                [&, key] { Delete(key); });
    deletion_queue_.emplace(key, id);
  }

  data_.at(CalcIndex(key)).push_back(Node{key, value, lifetime});

  return true;
}

HashTable::V HashTable::Get(const K& key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  for (Node node : data_.at(CalcIndex(key)))
    if (node.key == key) return node.value;

  return V{};
}

bool HashTable::Exists(const K& key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  for (Node node : data_.at(CalcIndex(key)))
    if (node.key == key) return true;

  return false;
}

bool HashTable::Delete(const K& key) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  size_t index = CalcIndex(key);
  for (Node node : data_.at(index)) {
    if (node.key == key) {
      auto item = deletion_queue_.find(key);
      if (item != deletion_queue_.end()) {
        pool_.StopTask(item->second);
        deletion_queue_.erase(item);
      }

      data_.at(index).remove(node);
      return true;
    }
  }
  return false;
}

bool HashTable::Update(const K& key, const V& value) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  for (Node& node : data_.at(CalcIndex(key))) {
    if (node.key == key) {
      node.value = value;
      return true;
    }
  }
  return false;
}

std::vector<HashTable::K> HashTable::Keys() const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  std::vector<K> result;
  for (std::list<Node> nodes : data_) {
    for (Node node : nodes) {
      result.push_back(node.key);
    }
  }
  return result;
}

bool HashTable::Rename(const K& from, const K& to) {
  if (Exists(to) && Exists(from) && from == to) return true;
  if (Exists(to)) return false;

  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  for (Node& node : data_.at(CalcIndex(from))) {
    if (node.key == from) {
      Set(to, node.value, Ttl(node.key));
      return Delete(from);
    }
  }
  return false;
}

int HashTable::Ttl(const K& key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  auto item = deletion_queue_.find(key);
  if (item != deletion_queue_.end())
    return pool_.GetRemainTime(item->second).count();

  return -1;
}

std::vector<HashTable::K> HashTable::Find(const V& value) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  std::vector<K> result;
  for (std::list<Node> nodes : data_) {
    for (Node node : nodes) {
      if (node.value == value) {
        result.push_back(node.key);
      }
    }
  }
  return result;
}

std::vector<HashTable::V> HashTable::ShowAll() const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  std::vector<V> result;
  for (std::list<Node> nodes : data_) {
    for (Node node : nodes) {
      result.push_back(node.value);
    }
  }
  return result;
}

int HashTable::Upload(const std::string& filename) {
  std::ifstream stream(filename);
  if (stream.is_open() == false) return 0;

  int number_of_lines = 0;
  K key;
  V value;
  while (stream >> key >> value) {
    Set(key, value);
    ++number_of_lines;
  }

  return number_of_lines;
}

int HashTable::Export(const std::string& filename) const {
  std::ofstream stream(filename);
  if (stream.is_open() == false) return 0;

  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  int number_of_lines = 0;
  for (std::list<Node> nodes : data_) {
    for (Node node : nodes) {
      stream << node.key << " " << node.value << std::endl;
      ++number_of_lines;
    }
  }

  return number_of_lines;
}

size_t HashTable::CalcHashCode(const K& key) const {
  size_t result = 0;
  size_t len = key.length();
  for (size_t i = 0; i < len; ++i) {
    result += key.at(i) * pow(31, len - i - 1);
  }
  return result;
}

size_t HashTable::CalcIndex(const K& key) const {
  return CalcHashCode(key) % capacity_;
}

}  // namespace s21
