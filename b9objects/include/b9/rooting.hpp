#if !defined B9_STACKROOTS_HPP_
#define B9_STACKROOTS_HPP_

namespace b9 {

class Cell;
class Context;

template <typename T>
class RootRef;

class RootRefSeq;

/// A rooted GC pointer. T must be a managed allocation. This class must be
/// stack allocated. RootRefs push themselves onto the linked list of stack
/// roots at construction. Because of this mechanism, RootRefs have a strict
/// LIFO lifetime.
template <typename T>
class RootRef {
 public:
  RootRef(Context& cx, T* ptr) noexcept;

  RootRef(RootRefSeq& seq, T* ptr) noexcept;

  // RootRefs must be stack allocated.
  void* operator new(std::size_t) = delete;

  RootRef(const RootRef& other) = delete;

  RootRef(RootRef&& other) = delete;

  ~RootRef() noexcept;

  T* ptr() const noexcept { return ptr_; }

  T* operator->() const noexcept { return ptr_; }

  T& operator*() const noexcept { return *ptr_; }

  RootRef& operator=(T* ptr) noexcept {
    ptr_ = ptr;
    return *this;
  }

  RootRef<Cell>* tail() const noexcept { return tail_; }

 private:
  T* ptr_;
  RootRefSeq& seq_;
  RootRef<Cell>* tail_;
};

class RootRefSeq {
 public:
  class Iterator {
   public:
    Iterator(RootRef<Cell>* head) : ptr_(head) {}

    RootRef<Cell>& operator*() const { return *ptr_; }

    Iterator& operator++() {
      ptr_ = ptr_->tail();
      return *this;
    }

    bool operator==(const Iterator& rhs) const { return ptr_ == rhs.ptr_; }

    bool operator!=(const Iterator& rhs) const { return ptr_ != rhs.ptr_; }

   private:
    RootRef<Cell>* ptr_;
  };

  class ConstIterator {
   public:
    ConstIterator(RootRef<Cell>* head) : ptr_(head) {}

    const RootRef<Cell>& operator*() const { return *ptr_; }

    ConstIterator& operator++() {
      ptr_ = ptr_->tail();
      return *this;
    }

    bool operator==(const ConstIterator& rhs) const { return ptr_ == rhs.ptr_; }

    bool operator!=(const ConstIterator& rhs) const { return ptr_ != rhs.ptr_; }

   private:
    RootRef<Cell>* ptr_;
  };

  RootRefSeq() : head_(nullptr) {}

  Iterator begin() { return Iterator(head_); }

  Iterator end() { return Iterator(nullptr); }

  ConstIterator begin() const { return cbegin(); }

  ConstIterator end() const { return cend(); }

  ConstIterator cbegin() const { return ConstIterator(head_); }

  ConstIterator cend() const { return ConstIterator(nullptr); }

  RootRef<Cell>* head() const noexcept { return head_; }

 protected:
  template <typename T>
  friend class RootRef;

  RootRefSeq& head(RootRef<Cell>* h) {
    head_ = h;
    return *this;
  }

 private:
  RootRef<Cell>* head_;
};

}  // namespace b9

#if 0
template <typename T, typename U = T>
using Cons = std::pair<T, Cons<U>*>;

namespace std {

template <typename T, typename U>
reinterpret_pointer_cast(RootRef<T>&) {}

template <typename T>
RootRef : public StackRoot<T> {}

template <>
RootRef : public StackRoot<T> {}

class Root {
  virtual void operator()(Visitor& visitor) = 0;
};

using RootTable = std::vector<Root>;

class RootTable {};
}  // namespace std

#endif  // 0

#endif  // B9_STACKROOTS_HPP_