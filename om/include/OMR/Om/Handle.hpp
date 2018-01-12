#if !defined(OMR_OM_HANDLE_HPP_)
#define OMR_OM_HANDLE_HPP_

#include <OMR/Om/RootRef.hpp>

namespace OMR {
namespace Om {

template <typename T>
class Handle {
 public:
  template <typename U>
  constexpr Handle(Handle<U> other)
      : root_(reinterpret_cast<T* const*>(other.root_)) {}

  template <typename U>
  explicit constexpr Handle(const RootRef<U>& root)
      : root_(reinterpret_cast<T**>(root.raw())) {}

  T* get() const noexcept { return *root_; }

  T* const* raw() const { return root_; }

  T& operator*() const { return *ptr(); }

  T* operator->() const noexcept { return ptr(); }

  template <typename U>
  U* operator->*(U T::*mptr) const noexcept {
    return ptr()->*mptr;
  }

  template <typename U>
  Handle<U> reinterpret() const {
    return Handle<U>(reinterpret_cast<U* const*>(root_));
  }

  template <typename U = T>
  constexpr U* ptr() const {
    return *static_cast<U* const*>(root_);
  }

  template <typename U = T>
  constexpr operator U*() const {
      return ptr<U>();
  }

 private:
  T* const* root_;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_HANDLE_HPP_
