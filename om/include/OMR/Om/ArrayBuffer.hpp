#if !defined(OMR_OM_ARRAYBUFFER_HPP_)
#define OMR_OM_ARRAYBUFFER_HPP_

#include <OMR/Infra/HashUtilities.hpp>
#include <OMR/Om/ArrayBufferMap.hpp>

namespace OMR {
namespace Om {

/// A managed cell of memory providing general-purpose raw storage.
/// Note that the ArrayBuffer is considered a leaf-object, references stored
/// into the buffer must be explicitly marked.

template <typename T = Cell>
struct ArrayBuffer {

  // TODO
  static ArrayBuffer<T>* allocate(Context& cx, std::size_t size) {
    return nullptr;
  }

  // TODO
  static ArrayBuffer<T>* reallocate(Context& cx, ArrayBuffer<T>* self,
                                    std::size_t size) {
    return nullptr;
  }

  static T& get(Context& cx, ArrayBuffer<T>* self, std::size_t index) {
    assert(index < self->size_);
    return self->data_[index];
  }

  static const T& get(Context& cx, const ArrayBuffer<T>* self,
                      std::size_t index) {
    assert(index < self->size_);
    return self->data_[index];
  }

  /// Obtain the underlying data pointer.
  static T* data(Context& cx, ArrayBuffer<T>* self) { return self->data_; }

  /// The array buffer has a generic method for calculating a hash of the data.
  /// In general, it is preferable that the user provides a less-general hashing
  /// function.
  static std::size_t hash(Context& cx, const ArrayBuffer<T>* self) {
    std::size_t hash = self->size_ + 1;
    for (std::size_t i = 0; i < self->size_; i++) {
      hash = Infra::Hash::mix(hash, self->data_[i]);
    }
    return hash;
  }

  std::size_t size_;
  T data_[0];
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_DATA_HPP_
