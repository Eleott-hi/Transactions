#include "b_plus_tree.h"

#include <algorithm>
#include <fstream>
#include <functional>

namespace s21 {

// ============================= Utils ===============================
namespace Utils {
template <class Type, class Cond>
auto GetPointerTo(std::vector<Type>& vec, Cond const& func) {
  std::function<bool(Type&)> condition = func;
  return std::find_if(vec.begin(), vec.end(), condition);
}

template <class Type, class Cond>
auto GetDistanceTo(std::vector<Type>& vec, Cond const& func) {
  auto itr = GetPointerTo(vec, func);
  return std::distance(vec.begin(), itr);
}

template <class Type>
void Append(std::vector<Type>& left, std::vector<Type>& right) {
  left.insert(left.end(), right.begin(), right.end());
}

}  // namespace Utils

//============================ Leaf =============================
bool BPlusTree::Leaf::Insert(K const& key, const V& value) {
  if (IsKeyExist(key)) return false;
  auto distance =
      Utils::GetDistanceTo(keys, [&](auto const& i) { return key < i; });
  keys.emplace(keys.begin() + distance, key);
  data.emplace(data.begin() + distance, std::make_shared<V>(value));
  return true;
}

BPlusTree::NodePtr BPlusTree::Leaf::Split() {
  auto leaf = std::make_shared<Leaf>();

  size_t mid = keys.size() / 2;

  leaf->keys = std::vector<K>(keys.begin() + mid, keys.end());
  leaf->data = std::vector<DataPtr>(data.begin() + mid, data.end());
  keys = std::vector<K>(keys.begin(), keys.begin() + mid);
  data = std::vector<DataPtr>(data.begin(), data.begin() + mid);

  leaf->next = next;
  next = leaf;

  return leaf;
}

void BPlusTree::Leaf::Merge(NodePtr right_node) {
  auto right = BPlusTree::CastNode<Leaf>(right_node);
  auto w_parent = parent.lock();

  // assert(w_parent);

  auto distance = Utils::GetDistanceTo(
      w_parent->children, [&](auto const& i) { return i == right; });

  w_parent->keys.erase(w_parent->keys.begin() + distance - 1);
  w_parent->children.erase(w_parent->children.begin() + distance);

  Utils::Append(keys, right->keys);
  Utils::Append(data, right->data);

  next = right->next;
}

bool BPlusTree::Leaf::IsKeyExist(K const& key) {
  return Utils::GetPointerTo(keys, [&](auto const& i) { return i == key; }) !=
         keys.end();
}

BPlusTree::V& BPlusTree::Leaf::GetValue(K const& key) {
  // assert(IsKeyExist(key));

  auto distance =
      Utils::GetDistanceTo(keys, [&](auto const& i) { return i == key; });

  return *data.at(distance);
}

void BPlusTree::Leaf::Delete(K const& key) {
  auto distance =
      Utils::GetDistanceTo(keys, [&](auto const& i) { return i == key; });
  keys.erase(keys.begin() + distance);
  data.erase(data.begin() + distance);
}

void BPlusTree::Leaf::Share(NodePtr node_from, NodePtr left_node) {
  auto from = BPlusTree::CastNode<Leaf>(node_from);
  auto left = BPlusTree::CastNode<Leaf>(left_node);
  auto w_parent = parent.lock();

  // assert(w_parent);

  K const& key = (from == left) ? from->keys.back() : from->keys.front();
  LeafPtr l_left = (from == left) ? from : shared_from_this();
  LeafPtr l_right = (from == left) ? shared_from_this() : from;

  Insert(key, from->GetValue(key));
  from->Delete(key);
  auto distance = Utils::GetDistanceTo(
      w_parent->children, [&](auto const& i) { return i == l_left; });
  w_parent->keys.at(distance) = l_right->keys.front();
}

//============================ Internal =============================

void BPlusTree::Internal::Insert(K const& key,  //
                                 NodePtr node,  //
                                 bool after_key) {
  auto distance =
      Utils::GetDistanceTo(keys, [&](auto const& i) { return key < i; });
  node->parent = shared_from_this();
  children.emplace(children.begin() + distance + after_key, node);
  keys.emplace(keys.begin() + distance, key);
}

void BPlusTree::Internal::Delete(K const& key, bool after_key) {
  auto distance =
      Utils::GetDistanceTo(keys, [&](auto const& i) { return key == i; });
  children.erase(children.begin() + distance + after_key);
  keys.erase(keys.begin() + distance);
}

BPlusTree::NodePtr BPlusTree::Internal::Split() {
  auto internal = std::make_shared<Internal>();

  size_t mid = keys.size() / 2;

  internal->keys = std::vector<K>(keys.begin() + mid, keys.end());
  keys = std::vector<K>(keys.begin(), keys.begin() + mid);
  internal->children =
      std::vector<NodePtr>(children.begin() + mid + 1, children.end());
  children = std::vector<NodePtr>(children.begin(), children.begin() + mid + 1);

  for (auto& i : internal->children) i->parent = internal;

  return internal;
}

void BPlusTree::Internal::Merge(NodePtr node_right) {
  auto right = BPlusTree::CastNode<Internal>(node_right);
  auto w_parent = parent.lock();

  // assert(w_parent);
  // assert(w_parent == right->parent.lock());

  auto distance = Utils::GetDistanceTo(
      w_parent->children, [&](auto const& i) { return i == right; });

  keys.push_back(w_parent->keys.at(distance - 1));

  Utils::Append(keys, right->keys);
  Utils::Append(children, right->children);

  std::for_each(children.begin(), children.end(),
                [&](auto const& i) { i->parent = shared_from_this(); });

  w_parent->Delete(*(w_parent->keys.begin() + distance - 1));
}

void BPlusTree::Internal::Share(NodePtr node_from, NodePtr node_left) {
  auto from = BPlusTree::CastNode<Internal>(node_from);
  auto left = BPlusTree::CastNode<Internal>(node_left);
  auto w_parent = parent.lock();

  // assert(w_parent);

  K const& key = (from == left) ? from->keys.back() : from->keys.front();
  NodePtr child =
      (from == left) ? from->children.back() : from->children.front();
  InternalPtr l_left = (from == left) ? from : shared_from_this();

  auto distance = Utils::GetDistanceTo(
      w_parent->children, [&](auto const& i) { return i == l_left; });
  Insert(w_parent->keys.at(distance), child, !(from == left));
  w_parent->keys.at(distance) = key;
  from->Delete(key, (from == left));
};

// ============================= BPlusTree ===============================
bool BPlusTree::Set(K const& key, const V& value, int lifetime) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  auto leaf = GetLeaf(root_, key);
  if (!leaf->Insert(key, value)) return false;

  if (leaf->Size() > bucket_size_) {
    auto new_leaf = leaf->Split();
    ShiftLevel(leaf, new_leaf, new_leaf->keys.front());
  }

  if (lifetime > -1) {
    size_t ID = pool_.DelayTask(std::chrono::seconds(lifetime),
                                [&, key] { Delete(key); });
    delay_deletions_.emplace(key, ID);
  }

  ++size_;
  return true;
}

BPlusTree::V BPlusTree::Get(K const& key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  auto leaf = GetLeaf(root_, key);
  if (leaf->IsKeyExist(key)) return leaf->GetValue(key);
  return {};
}

bool BPlusTree::Exists(K const& key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  auto leaf = GetLeaf(root_, key);
  return leaf->IsKeyExist(key);
}

bool BPlusTree::Delete(K const& key) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  auto itr = delay_deletions_.find(key);
  if (itr != delay_deletions_.end()) {
    pool_.StopTask(itr->second);
    delay_deletions_.erase(itr);
  }

  auto leaf = GetLeaf(root_, key);
  if (!leaf->IsKeyExist(key)) return false;

  leaf->Delete(key);
  UpdateTree(leaf);

  --size_;
  return true;
}

bool BPlusTree::Update(K const& key, V const& value) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  auto leaf = GetLeaf(root_, key);
  if (leaf->IsKeyExist(key)) {
    leaf->GetValue(key) = value;
    return true;
  }
  return false;
}

std::vector<BPlusTree::K> BPlusTree::Keys() const {
  std::vector<BPlusTree::K> res;
  res.reserve(size_);

  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (auto leaf = list_; leaf; leaf = leaf->next)
    res.insert(res.end(), leaf->keys.cbegin(), leaf->keys.cend());

  return res;
}

bool BPlusTree::Rename(K const& from, K const& to) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  auto leaf = GetLeaf(root_, from);
  if (!leaf->IsKeyExist(from)) return false;
  if (from == to) return true;

  bool res = Set(to, leaf->GetValue(from), Ttl(from));
  if (res) Delete(from);
  return res;
}

int BPlusTree::Ttl(K const& key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  auto itr = delay_deletions_.find(key);
  if (itr != delay_deletions_.end())
    return pool_.GetRemainTime(itr->second).count();

  return -1;
}

std::vector<BPlusTree::K> BPlusTree::Find(const V& value) const {
  std::vector<BPlusTree::K> res;
  res.reserve(size_);

  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (auto leaf = list_; leaf; leaf = leaf->next)
    std::copy_if(leaf->keys.begin(), leaf->keys.end(), std::back_inserter(res),
                 [&](auto const& i) { return leaf->GetValue(i) == value; });

  return res;
}

std::vector<BPlusTree::V> BPlusTree::ShowAll() const {
  std::vector<BPlusTree::V> res;
  res.reserve(size_);

  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (auto leaf = list_; leaf; leaf = leaf->next)
    std::transform(leaf->data.cbegin(), leaf->data.cend(),
                   std::back_inserter(res), [](auto const& i) { return *i; });

  return res;
}

int BPlusTree::Upload(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return 0;
  }

  int res = 0;
  K key;
  V value;
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  while (file >> key >> value) {
    Set(key, value);
    ++res;
  }

  return res;
}

int BPlusTree::Export(const std::string& filename) const {
  std::ofstream file(filename);
  if (!file.is_open()) {
    return 0;
  }

  int res = 0;
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (auto leaf = list_; leaf; leaf = leaf->next)
    for (auto& key : leaf->keys) {
      file << key << " " << leaf->GetValue(key) << "\n";
      ++res;
    }

  return res;
}

//============================ PRIVATE =============================

std::tuple<BPlusTree::NodePtr, BPlusTree::NodePtr> BPlusTree::GetSiblings(
    NodePtr node) {
  // assert(node);

  NodePtr left = nullptr, right = nullptr;

  auto parent = node->parent.lock();
  if (parent) {
    auto itr = Utils::GetPointerTo(
        parent->keys, [&](auto const& i) { return node->keys.front() < i; });

    if (itr != parent->keys.begin()) {
      size_t distance = std::distance(parent->keys.begin(), itr - 1);
      left = parent->children.at(distance);
    }

    if (itr != parent->keys.end()) {
      size_t distance = std::distance(parent->keys.begin(), itr + 1);
      right = parent->children.at(distance);
    }
  }

  return {left, right};
}

BPlusTree::LeafPtr BPlusTree::GetLeaf(NodePtr node, K const& key) const {
  // assert(node);

  if (node->IsLeaf()) return CastNode<Leaf>(node);

  auto cast_node = CastNode<Internal>(node);
  auto& keys = cast_node->keys;
  auto& children = cast_node->children;

  for (size_t i = 0; i < keys.size(); ++i)
    if (key < keys[i]) return GetLeaf(children[i], key);

  return GetLeaf(children.back(), key);
}

void BPlusTree::UpdateTree(NodePtr node) {
  // assert(node);

  if (node == root_ && !node->IsLeaf() && !node->Size()) {
    auto cast_node = CastNode<Internal>(node);
    // assert(cast_node->children.size() == 1);

    root_ = cast_node->children.front();
    root_->parent.reset();
    return;
  }

  if (node == root_ || node->Size() >= (bucket_size_ + !node->IsLeaf()) / 2)
    return;

  auto [left, right] = GetSiblings(node);

  if (left && left->Size() > (bucket_size_ + 1) / 2) {
    node->Share(left, left);
  } else if (right && right->Size() > (bucket_size_ + 1) / 2) {
    node->Share(right, node);
  } else if (left) {
    left->Merge(node);
    UpdateTree(left->parent.lock());
  } else if (right) {
    node->Merge(right);
    UpdateTree(node->parent.lock());
  } else {
    // assert(false);
  }
}

void BPlusTree::ShiftLevel(NodePtr left, NodePtr right, K const& key) {
  if (left == root_) {
    auto new_root = std::make_shared<Internal>();
    left->parent = right->parent = new_root;
    new_root->children = {left, right};
    new_root->keys.push_back(key);
    root_ = new_root;

  } else {
    auto internal = left->parent.lock();
    // assert(internal);

    internal->Insert(key, right);
    if (internal->Size() <= bucket_size_) return;

    auto new_internal = internal->Split();
    K middle_key = new_internal->keys.front();
    new_internal->keys.erase(new_internal->keys.begin());
    ShiftLevel(internal, new_internal, middle_key);
  }
}

}  // namespace s21
