#if !defined(OMR_OM_ROOTREF_HPP_)
#define OMR_OM_ROOTREF_HPP_

#include <OMR/Infra/Cons.hpp>

#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>

namespace OMR {
namespace Om {

class Cell;

/// The RefSeq is a sequence of consed GC References. The cons are unowned data,
/// they must be manually allocated and freed.
class RefSeq {
 public:
  using Node = Infra::Cons<Cell*>;
  using Iterator = Node::Iterator;
  using ConstIterator = Node::ConstIterator;

  RefSeq() : head_(nullptr) {}

  Iterator begin() { return head_->begin(); }

  Iterator end() { return head_->end(); }

  ConstIterator begin() const noexcept { return cbegin(); }

  ConstIterator end() const noexcept { return cend(); }

  ConstIterator cbegin() const noexcept { return head_->cbegin(); }

  ConstIterator cend() const noexcept { return head_->cend(); }

  Node* head() const noexcept { return head_; }

  RefSeq& head(Node* h) {
    head_ = h;
    return *this;
  }

 private:
  Node* head_;
};

class Context;

/// A sequence of RootRef objects. RootRefSeq is a strongly typed RefSeq, with
/// no additional APIs.
class RootRefSeq : public RefSeq {
  using RefSeq::RefSeq;
};

/// A rooted GC pointer. T must be a managed allocation. This class must be
/// stack allocated. RootRefs push themselves onto the linked list of stack
/// roots at construction. When A GC occurs, the sequence of RootRefs will be
/// walked, the referants marked, and the references fixed up. RootRefs have a
/// strict LIFO lifetime, when a RootRef is destroyed, it must be the head of
/// the list.
template <typename T>
class RootRef {
 public:
  inline RootRef(Context& cx, T* ptr = nullptr) noexcept;

  inline RootRef(RootRefSeq& seq, T* ptr = nullptr) noexcept;

  /// Copy from other root ref. The resulting rootref is pushed onto the head of
  /// other's sequence.
  template <typename U>
  inline RootRef(const RootRef<U>& other) noexcept;

  /// Move from other root ref. The RootRef we are moving from must be the head
  /// of the RefSeq. The moved-from RootRef is invalidated and cleared.
  template <typename U>
  inline RootRef(RootRef<U>&& other) noexcept;

  // RootRefs must be stack allocated.
  void* operator new(std::size_t) = delete;

  inline ~RootRef() noexcept;

  /// Obtain the underlying pointer.
  template <typename U = T>
  U* get() const noexcept {
    return reinterpret_cast<U*>(node_.first);
  }

  T* operator->() const noexcept { return get(); }

  T& operator*() const noexcept { return *get(); }

  RootRef& operator=(T* p) noexcept {
    node_.first = p;
    return *this;
  }

  bool isHead() const noexcept { return seq_.head() == &node_; }

  RefSeq& seq() const noexcept { return seq_; }

  RefSeq::Node* tail() const noexcept { return node_.second; }

 private:
  RefSeq& seq_;
  RefSeq::Node node_;
};

template <typename T, typename U>
T get(RootRef<U>& root) {
  return root.template get<T>();
}

// TODO: Find new home for the MarkingFn APIs.

class Visitor;

using MarkingFn = std::function<void(Context&, Visitor&)>;

using MarkingFnVector = std::vector<MarkingFn>;

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ROOTREF_HPP_
