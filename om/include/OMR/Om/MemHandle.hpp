#if !defined(OMR_OM_MEMHANDLE_HPP_)
#define OMR_OM_MEMHANDLE_HPP_

#include <OMR/Infra/PointerUtilities.hpp>
#include <OMR/Om/Handle.hpp>

namespace OMR {
namespace Om {

class Cell;

/// A Member Handle. A handle to a field inside a cell. A MemHandle has two
/// parts: a handle to the cell, and an offset into that cell. This class acts
/// as a handle, which ensures that the base Cell is rooted, and is safe to hold
/// across GC operations. See also: OMR::Om::Ref, OMR::Om::Root, OMR::Om::Handle, OMR::Om::MemRef.
template <typename T, typename C = Cell>
class MemHandle {
 public:
  template <typename U = C>
  MemHandle(Handle<U> base, T U::*p)
      : base_(base), offset_(Infra::ptrdiff(base.ptr(), &(base->*p))) {}

  /// Construct a memb
  template <typename U = C>
  MemHandle(const Handle<U>& base, std::uintptr_t offset)
      : base_(base), offset_(offset) {}

  /// Copy a Member Handle
  template <typename U>
  MemHandle(const MemHandle<U>& other)
      : base_(other.base_), offset_(other.offset_) {}

  /// Construct a nested member from another member handle.
  template <typename U, typename CU>
  MemHandle(const MemHandle<U, CU>& inner, std::uintptr_t offset)
      : base_(inner.base_), offset_(inner.offset_ + offset) {}

  /// Construct a nested member from another member handle.
  template <typename U>
  MemHandle(const MemHandle& inner, T U::*member)
      : base_(inner.base_),
        offset_(Infra::ptrdiff(inner.base(), &inner->*member)) {}

  T* operator->() const { return Infra::ptradd<T>(base_, offset_); }

  T& operator*() const { return *Infra::ptradd<T>(base_, offset_); }

  constexpr Handle<C> base() const { return base_; }

  MemHandle<T, C>& base(Handle<C>& base) {
    base_ = &base_;
    return *this;
  }

  constexpr operator T*() const {
    return Infra::ptradd<C, T>(base_.ptr(), offset_);
  }

 private:
  Handle<C> base_;
  std::ptrdiff_t offset_;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MEMHANDLE_HPP_
