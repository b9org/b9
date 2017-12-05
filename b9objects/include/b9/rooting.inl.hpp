#if !defined(B9_ROOTING_INL_HPP_)
#define B9_ROOTING_INL_HPP_

#include <b9/context.inl.hpp>
#include <b9/rooting.hpp>

namespace b9 {

template <typename T>
inline RootRef<T>::RootRef(Context& cx, T* ptr) noexcept
    : RootRef(cx.stackRoots(), ptr) {}

template <typename T>
inline RootRef<T>::RootRef(RootRefSeq& seq, T* ptr) noexcept
    :  seq_(seq), node_(ptr, seq.head()) {
  seq_.head(&node_);
}

template <typename T>
template <typename U>
inline RootRef<T>::RootRef(const RootRef<U>& other) noexcept
  : RootRef(other.seq(), get<T*>(other)) {
}

template <typename T>
template <typename U>
inline RootRef<T>::RootRef(RootRef<U>&& other) noexcept
  : seq_(other.seq()), node_(get<T*>(other), other.tail()) {
    assert(other.isHead());
    other.node_.first = nullptr;
    other.node_.second = nullptr;
    seq_.head(&node_);
  }

template <typename T>
inline RootRef<T>::~RootRef() noexcept {
  assert(isHead());
  seq_.head(node_.second);
}

}  // namespace b9

#endif  // B9_ROOTING_INL_HPP_
