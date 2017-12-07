#if !defined B9_STACKROOTS_HPP_
#define B9_STACKROOTS_HPP_

#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>

namespace b9 {

/// Linked list element. A specialization of std::pair that supports forward
/// iteration. cons.first is an instance of T. cons.second is a pointer to the
/// next.
template <typename T>
class Cons : public std::pair<T, Cons<T>*> {
 public:
  class Iterator {
   public:
    Iterator() : current_(nullptr) {}

    Iterator(Cons<T>* head) : current_(head) {}

    Iterator(const Iterator& other) : current_(other.current_) {}

    T& operator*() const { return current_->first; }

    T* operator->() const { return &current_->first; }

    Iterator& operator++() {
      current_ = current_->second;
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp(*this);
      current_ = current_->second;
      return tmp;
    }

    bool operator==(const Iterator& rhs) const {
      return current_ == rhs.current_;
    }

    bool operator!=(const Iterator& rhs) const {
      return current_ != rhs.current_;
    }

   private:
    Cons<T>* current_;
  };

  class ConstIterator {
   public:
    ConstIterator() : current_(nullptr) {}

    ConstIterator(const Cons<T>* head) : current_(head) {}

    ConstIterator(const Iterator& other) : current_(other.current_) {}

    const T& operator*() const { return current_->first; }

    const T* operator->() const { return &current_->first; }

    ConstIterator& operator++() {
      current_ = current_->second;
      return *this;
    }

    ConstIterator operator++(int) {
      Iterator tmp(*this);
      current_ = current_->second;
      return tmp;
    }

    bool operator==(const ConstIterator& rhs) const {
      return current_ == rhs.current_;
    }

    bool operator!=(const ConstIterator& rhs) const {
      return current_ != rhs.current_;
    }

   private:
    const Cons<T>* current_;
  };

  using std::pair<T, Cons<T>*>::pair;

  Iterator begin() noexcept { return Iterator(this); }

  Iterator end() noexcept { return Iterator(); }

  ConstIterator begin() const noexcept { return cbegin(); }

  ConstIterator end() const noexcept { return cend(); }

  ConstIterator cbegin() const noexcept { return ConstIterator(this); }

  ConstIterator cend() const noexcept { return ConstIterator(); }
};

class Cell;

/// The RefSeq is a sequence of consed GC References. The cons are unowned data,
/// they must be manually allocated and freed.
class RefSeq {
 public:
  using Node = Cons<Cell*>;
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

/// A sequence of RootRef objects.
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

class Visitor;

using MarkingFn = std::function<void(Context&, Visitor&)>;

using MarkingFnVector = std::vector<MarkingFn>;

}  // namespace b9

#endif  // B9_STACKROOTS_HPP_