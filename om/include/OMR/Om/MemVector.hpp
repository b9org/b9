#if !defined(OMR_OM_MEMVECTOR_HPP_)
#define OMR_OM_MEMVECTOR_HPP_

#include <OMR/Om/ArrayBuffer.hpp>
#include <OMR/Om/MemHandle.hpp>
#include <cstdint>

namespace OMR {
namespace Om {

/// A vector-API backed by a heap-allocated ArrayBuffer Cell. As a Member-type,
/// vectors are not self contained objects, but are meant to be embedded in
/// other Cell types.
template <typename T>
class MemVector {
 public:
  static std::size_t capacity(Context& cx, const MemVector<T>* self) {
    return self->buffer_->size / sizeof(T);
  }

  static std::size_t size(Context& cx, const MemVector<T>* self) {
    return self->size_;
  }

  // TODO: Barrier::write(cx, handle.base(), &handle->buffer, newBuffer);
  static bool resize(Context& cx, const MemHandle<MemVector<T>>& self,
                     std::size_t size) {
    auto newBuffer = ArrayBuffer<T>::reallocate(cx, self->buffer_, size);
    if (newBuffer != nullptr) {
      self->buffer_ = newBuffer;
      return true;
    }
    return false;
  }

  static T& get(Context& cx, const MemVector<T>* self, std::size_t index) {
    return ArrayBuffer<T>::get(cx, self->buffer, index);
  }

  // TODO: Barrier::store(cx, self, &Member::buffer, buffer);
  static void construct(Context& cx, const MemHandle<MemVector<T>>& self,
                        std::size_t size = 0) {
    self->buffer_ = ArrayBuffer<T>::allocate(cx, size);
    self->size_ = size;
  }

 private:
  std::size_t size_;
  ArrayBuffer<T>* buffer_;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MEMVECTOR_HPP_
