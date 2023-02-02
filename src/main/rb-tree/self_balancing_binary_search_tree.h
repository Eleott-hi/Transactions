#ifndef A6_SRC_MAIN_RB_TREE_SELF_BALANCING_BINARY_SEARCH_TREE_H_
#define A6_SRC_MAIN_RB_TREE_SELF_BALANCING_BINARY_SEARCH_TREE_H_

#include <map>
#include <memory>

#include "async_pool.h"
#include "key_value_storage.h"

namespace s21 {

class SelfBalancingBinarySearchTree : public KeyValueStorage {
 public:
  bool Set(K const& key, const V& value, int lifetime = -1) override;
  [[nodiscard]] std::vector<K> Find(const V& value) const override;
  [[nodiscard]] bool Exists(K const& key) const override;
  int Upload(const std::string& filename) override;
  int Export(const std::string& filename) const override;
  [[nodiscard]] std::vector<V> ShowAll() const override;
  [[nodiscard]] std::vector<K> Keys() const override;
  bool Rename(K const& from, K const& to) override;
  [[nodiscard]] int Ttl(K const& key) const override;
  bool Update(K const& key, V const& value) override;
  [[nodiscard]] V Get(K const& key) const override;
  bool Delete(K const& key) override;

 private:
  enum class NodeColor { kRed, kBlack };
  struct Node;
  friend struct Node;
  using NodePtr = std::shared_ptr<Node>;

  size_t size_ = 0;
  AsyncPool pool_;
  NodePtr root_ = nullptr;
  mutable std::recursive_mutex mtx_;
  std::map<K, size_t> delay_deletions_;

  bool Insert(NodePtr node, K const& key, const V& value);
  NodePtr GetNode(NodePtr node, K const& key) const;
  NodePtr NextNode(NodePtr node = nullptr) const;
  void Rotation(NodePtr node, bool right);
  void InsertionCheck(NodePtr node);
  void CheckRotation(NodePtr node);
  void DeleteNode(NodePtr node);
  void Recolor(NodePtr node) const;
  void DeletionCheck(NodePtr node);
};

struct SelfBalancingBinarySearchTree::Node
    : public std::enable_shared_from_this<Node> {
  K key;
  V value;
  std::weak_ptr<Node> parent;
  NodeColor color;
  NodePtr left;
  NodePtr right;

  Node(K const& key, V const& value, NodePtr const& parent = nullptr,
       NodeColor color = NodeColor::kRed, NodePtr const& left = nullptr,
       NodePtr const& right = nullptr);

  NodePtr GetNephew(bool far);
  NodePtr GetParent() { return parent.lock(); }
  NodePtr GetSibling();

  bool IsRightChild();
  bool IsLeftChild();
  bool IsRed() { return color == NodeColor::kRed; }

  void ReplaceChild(NodePtr old_value, NodePtr new_value);
  void Swap(NodePtr other);
  bool AreChildrenBlack();
};

}  // namespace s21

#endif  // A6_SRC_MAIN_RB_TREE_SELF_BALANCING_BINARY_SEARCH_TREE_H_
