#ifndef A6_SRC_MAIN_BP_TREE_B_PLUS_TREE_H_
#define A6_SRC_MAIN_BP_TREE_B_PLUS_TREE_H_

#include <map>
#include <memory>
#include <tuple>

#include "async_pool.h"
#include "key_value_storage.h"

namespace s21 {

// ============================ B + Tree ==============================
class BPlusTree : public KeyValueStorage {
 public:
  BPlusTree() = default;
  ~BPlusTree() {
    list_ = nullptr;
    root_ = nullptr;
  }
  BPlusTree(const BPlusTree&) = delete;
  BPlusTree(BPlusTree&&) = delete;
  void operator=(const BPlusTree&) = delete;
  void operator=(BPlusTree&&) = delete;

  bool Set(K const& key, const V& value, int lifetime = -1) override;
  [[nodiscard]] std::vector<K> Find(const V& value) const override;
  [[nodiscard]] bool Exists(K const& key) const override;
  int Upload(const std::string& filename) override;
  int Export(const std::string& filename) const override;
  [[nodiscard]] std::vector<V> ShowAll() const override;
  [[nodiscard]] std::vector<K> Keys() const override;
  bool Rename(K const& from, K const& to) override;
  [[nodiscard]] int Ttl(K const& key) const override;
  [[nodiscard]] V Get(K const& key) const override;
  bool Update(K const& key, V const& value) override;
  bool Delete(K const& key) override;

 private:
  struct Node;
  struct Internal;
  struct Leaf;

  using InternalPtr = std::shared_ptr<Internal>;
  using NodePtr = std::shared_ptr<Node>;
  using LeafPtr = std::shared_ptr<Leaf>;
  using DataPtr = std::shared_ptr<V>;

  LeafPtr list_ = std::make_shared<Leaf>();
  NodePtr root_ = std::static_pointer_cast<Node>(list_);
  std::map<K, size_t> delay_deletions_;
  mutable std::recursive_mutex mtx_;
  size_t bucket_size_ = 10;
  size_t size_ = 0;
  AsyncPool pool_;

  template <typename Type>
  static std::shared_ptr<Type> CastNode(NodePtr node) {
    return std::static_pointer_cast<Type>(node);
  }

  void ShiftLevel(NodePtr left, NodePtr right, K const& key);
  LeafPtr GetLeaf(NodePtr node, K const& key) const;

  std::tuple<BPlusTree::NodePtr, BPlusTree::NodePtr> GetSiblings(NodePtr node);
  void UpdateTree(NodePtr node);
};

// ============================ BASE_NODE ==============================
struct BPlusTree::Node {
  std::vector<K> keys;
  std::weak_ptr<Internal> parent;

  virtual void Share(NodePtr from, NodePtr left) = 0;
  virtual bool IsLeaf() const { return false; }
  size_t Size() const { return keys.size(); }
  virtual void Merge(NodePtr right) = 0;
  virtual NodePtr Split() = 0;
};

// ============================ INTERNAL ==============================
struct BPlusTree::Internal : public Node,
                             public std::enable_shared_from_this<Internal> {
  std::vector<NodePtr> children;

  virtual NodePtr Split() override;
  virtual void Merge(NodePtr right) override;
  void Delete(K const& key, bool after_key = true);
  virtual void Share(NodePtr from, NodePtr left) override;
  void Insert(K const& key, NodePtr node, bool after_key = true);
};

// =============================== LEAF ===============================
struct BPlusTree::Leaf : public Node,
                         public std::enable_shared_from_this<Leaf> {
  std::vector<DataPtr> data;
  LeafPtr next = nullptr;

  virtual void Share(NodePtr from, NodePtr left) override;
  virtual bool IsLeaf() const override { return true; }
  virtual void Merge(NodePtr right) override;
  bool Insert(K const& key, const V& value);
  virtual NodePtr Split() override;
  bool IsKeyExist(K const& key);
  V& GetValue(K const& key);
  void Delete(K const& key);
};

}  // namespace s21

#endif  // A6_SRC_MAIN_BP_TREE_B_PLUS_TREE_H_
