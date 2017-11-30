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
    : ptr_(ptr), seq_(seq), tail_(seq.head()) {
  seq_.head(reinterpret_cast<RootRef<Cell>*>(this));
}

template <typename T>
inline RootRef<T>::~RootRef() noexcept {
  assert(seq_.head() == reinterpret_cast<RootRef<Cell>*>(this));
  ptr_ = nullptr;
  tail_ = nullptr;
  seq_.head(tail_);
}

}  // namespace b9

#endif  // B9_ROOTING_INL_HPP_
