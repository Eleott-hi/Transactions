#include "self_balancing_binary_search_tree.h"

#include <fstream>

namespace s21 {

bool SelfBalancingBinarySearchTree::Set(K const &key, const V &value,
                                        int lifetime) {
  bool res = false;
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  if (!root_) {
    root_ = std::make_shared<Node>(key, value, nullptr, NodeColor::kBlack);
    res = true;
  } else {
    res = Insert(root_, key, value);
  }

  if (res && lifetime > -1) {
    size_t id = pool_.DelayTask(std::chrono::seconds(lifetime),
                                [&, key] { Delete(key); });
    delay_deletions_.emplace(key, id);
  }

  ++size_;
  return res;
}

SelfBalancingBinarySearchTree::V SelfBalancingBinarySearchTree::Get(
    K const &key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  NodePtr node = GetNode(root_, key);
  if (node) return node->value;

  return {};
}

bool SelfBalancingBinarySearchTree::Exists(K const &key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  return GetNode(root_, key) != nullptr;
}

bool SelfBalancingBinarySearchTree::Delete(K const &key) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  auto itr = delay_deletions_.find(key);
  if (itr != delay_deletions_.end()) {
    pool_.StopTask(itr->second);
    delay_deletions_.erase(itr);
  }

  NodePtr node = GetNode(root_, key);
  if (!node) return false;

  DeleteNode(node);

  --size_;

  return true;
}

bool SelfBalancingBinarySearchTree::Update(K const &key, V const &value) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);

  NodePtr node = GetNode(root_, key);
  if (node) {
    node->value = value;
    return true;
  }
  return false;
}

std::vector<SelfBalancingBinarySearchTree::K>
SelfBalancingBinarySearchTree::Keys() const {
  std::vector<K> res;
  res.reserve(size_);

  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (NodePtr node = NextNode(); node; node = NextNode(node))
    res.push_back(node->key);

  return res;
}

bool SelfBalancingBinarySearchTree::Rename(K const &from, K const &to) {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  NodePtr node = GetNode(root_, from);
  if (!node) return false;
  if (from == to) return true;

  bool res = Set(to, node->value, Ttl(from));
  if (res) Delete(from);
  return res;
}

int SelfBalancingBinarySearchTree::Ttl(K const &key) const {
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  auto itr = delay_deletions_.find(key);
  if (itr != delay_deletions_.end())
    return pool_.GetRemainTime(itr->second).count();

  return -1;
}

std::vector<SelfBalancingBinarySearchTree::K>
SelfBalancingBinarySearchTree::Find(const V &value) const {
  std::vector<K> res;
  res.reserve(size_);

  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (NodePtr node = NextNode(); node; node = NextNode(node))
    if (node->value == value) res.push_back(node->key);

  return res;
}

std::vector<SelfBalancingBinarySearchTree::V>
SelfBalancingBinarySearchTree::ShowAll() const {
  std::vector<V> res;
  res.reserve(size_);

  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (NodePtr node = NextNode(); node; node = NextNode(node))
    res.push_back(node->value);

  return res;
}

int SelfBalancingBinarySearchTree::Upload(const std::string &filename) {
  std::ifstream stream(filename);
  if (stream.is_open() == false) {
    return 0;
  }

  int res = 0;
  K key;
  V value;
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  while (stream >> key >> value) {
    Set(key, value);
    ++res;
  }

  return res;
}

int SelfBalancingBinarySearchTree::Export(const std::string &filename) const {
  std::ofstream file(filename);
  if (!file.is_open()) {
    return 0;
  }

  int res = 0;
  std::scoped_lock<std::recursive_mutex> lock(mtx_);
  for (NodePtr node = NextNode(); node; node = NextNode(node), ++res)
    file << node->key << " " << node->value << "\n";

  return res;
}

// ========================= PRIVATE ============================

void SelfBalancingBinarySearchTree::DeleteNode(NodePtr node) {
  if (!node) return;

  if (node->left && node->right) {
    NodePtr tmp = node->right;
    while (tmp->left) tmp = tmp->left;
    node->Swap(tmp);
    DeleteNode(tmp);

  } else if (node->left || node->right) {
    node->Swap(node->right ? node->right : node->left);
    DeleteNode(node->right ? node->right : node->left);

  } else {
    DeletionCheck(node);
    NodePtr parent = node->GetParent();

    if (parent) {
      parent->ReplaceChild(node, nullptr);

    } else {
      root_ = nullptr;
    }
  }
}

void SelfBalancingBinarySearchTree::DeletionCheck(NodePtr node) {
  if (!node || node == root_ || node->IsRed()) return;

  NodePtr sibling = node->GetSibling();
  NodePtr parent = node->GetParent();

  if (sibling && !sibling->IsRed()) {
    if (sibling->AreChildrenBlack()) {
      Recolor(sibling);

      if (parent->IsRed()) {
        parent->color = NodeColor::kBlack;
      } else {
        DeletionCheck(parent);
      }
    } else {
      NodePtr far_nephew = node->GetNephew(true);
      NodePtr near_nephew = node->GetNephew(false);

      if (far_nephew && far_nephew->IsRed()) {
        std::swap(sibling->color, parent->color);
        Recolor(far_nephew);
        Rotation(sibling, node->IsRightChild());
      } else {
        std::swap(near_nephew->color, sibling->color);
        Rotation(near_nephew, node->IsLeftChild());
        DeletionCheck(node);
      }
    }
  } else if (sibling) {
    std::swap(sibling->color, parent->color);
    Rotation(sibling, !node->IsLeftChild());
    DeletionCheck(node);
  }
}

SelfBalancingBinarySearchTree::NodePtr  //
SelfBalancingBinarySearchTree::GetNode(NodePtr node, K const &key) const {
  if (!node) return nullptr;

  if (key < node->key) {
    return GetNode(node->left, key);
  } else if (key > node->key) {
    return GetNode(node->right, key);
  } else {
    return node;
  }
}

bool SelfBalancingBinarySearchTree::Insert(NodePtr node, K const &key,
                                           const V &value) {
  if (key < node->key) {
    if (node->left) {
      return Insert(node->left, key, value);

    } else {
      node->left = std::make_shared<Node>(key, value, node);
      InsertionCheck(node->left);
      return true;
    }

  } else if (key > node->key) {
    if (node->right) {
      return Insert(node->right, key, value);

    } else {
      node->right = std::make_shared<Node>(key, value, node);
      InsertionCheck(node->right);
      return true;
    }

  } else {
    return false;
  }
}

void SelfBalancingBinarySearchTree::InsertionCheck(NodePtr node) {
  if (node == root_) return;
  NodePtr parent = node->GetParent();

  if (parent && parent->IsRed()) {
    NodePtr uncle = parent->GetSibling();
    if (uncle && uncle->IsRed()) {
      Recolor(parent);
      Recolor(uncle);

      NodePtr grandpa = parent->GetParent();
      if (grandpa && grandpa != root_) {
        Recolor(grandpa);
        InsertionCheck(grandpa);
      }
    } else {
      CheckRotation(node);
    }
  }
}

void SelfBalancingBinarySearchTree::CheckRotation(NodePtr node) {
  NodePtr parent = node->GetParent();
  if (node->IsRightChild() && parent->IsLeftChild()) {
    Rotation(node, false);
    Rotation(node, true);
    Recolor(node);
    Recolor(node->right);

  } else if (node->IsLeftChild() && parent->IsRightChild()) {
    Rotation(node, true);
    Rotation(node, false);
    Recolor(node);
    Recolor(node->left);

  } else if (node->IsRightChild() && parent->IsRightChild()) {
    node = parent;
    Rotation(node, false);
    Recolor(node);
    Recolor(node->left);

  } else if (node->IsLeftChild() && parent->IsLeftChild()) {
    node = parent;
    Rotation(node, true);
    Recolor(node);
    Recolor(node->right);
  }
}

void SelfBalancingBinarySearchTree::Rotation(NodePtr node, bool right) {
  NodePtr tmp = node->GetParent();
  NodePtr grandpa = tmp->GetParent();
  if (grandpa) {
    grandpa->ReplaceChild(tmp, node);
  } else {
    root_ = node;
  }

  node->parent = tmp->parent;
  tmp->parent = node;

  if (right) {
    tmp->left = node->right;
    if (node->right) node->right->parent = tmp;
    node->right = tmp;
  } else {
    tmp->right = node->left;
    if (node->left) node->left->parent = tmp;
    node->left = tmp;
  }
}

void SelfBalancingBinarySearchTree::Recolor(NodePtr node) const {
  node->color =
      (node->IsRed() || node == root_) ? NodeColor::kBlack : NodeColor::kRed;
}

SelfBalancingBinarySearchTree::NodePtr SelfBalancingBinarySearchTree::NextNode(
    NodePtr node) const {
  auto min_node = [&](NodePtr node) -> NodePtr {
    while (node && node->left) node = node->left;
    return node;
  };

  if (node == nullptr) {
    node = min_node(root_);

  } else if (node->right) {
    node = min_node(node->right);

  } else if (node->IsLeftChild()) {
    node = node->GetParent();

  } else if (node->IsRightChild()) {
    while (node->IsRightChild()) node = node->GetParent();
    node = node->GetParent();

  } else {
    node = node->GetParent();
  }

  return node;
}

SelfBalancingBinarySearchTree::Node::Node(K const &key, V const &value,
                                          NodePtr const &parent,
                                          NodeColor color, NodePtr const &left,
                                          NodePtr const &right)
    : key(key),
      value(value),
      parent(parent),
      color(color),
      left(left),
      right(right) {}

SelfBalancingBinarySearchTree::NodePtr
SelfBalancingBinarySearchTree::Node::GetNephew(bool far) {
  NodePtr sibling = GetSibling();
  if (!sibling) return nullptr;
  if (far) return IsLeftChild() ? sibling->right : sibling->left;
  return IsLeftChild() ? sibling->left : sibling->right;
}

SelfBalancingBinarySearchTree::NodePtr
SelfBalancingBinarySearchTree::Node::GetSibling() {
  auto parent = GetParent();
  if (IsRightChild()) return parent->left;
  if (IsLeftChild()) return parent->right;
  return nullptr;
}

bool SelfBalancingBinarySearchTree::Node::IsRightChild() {
  auto parent = GetParent();
  if (parent) return parent->right == shared_from_this();
  return false;
}

bool SelfBalancingBinarySearchTree::Node::IsLeftChild() {
  auto parent = GetParent();
  if (parent) return parent->left == shared_from_this();
  return false;
}

void SelfBalancingBinarySearchTree::Node::ReplaceChild(NodePtr old_value,
                                                       NodePtr new_value) {
  if (old_value == left)
    left = new_value;
  else if (old_value == right)
    right = new_value;
}

void SelfBalancingBinarySearchTree::Node::Swap(NodePtr other) {
  std::swap(key, other->key);
  std::swap(value, other->value);
}

bool SelfBalancingBinarySearchTree::Node::AreChildrenBlack() {
  return (!left || !left->IsRed()) && (!right || !right->IsRed());
}

}  // namespace s21
