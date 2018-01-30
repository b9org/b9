#if !defined(OMR_OM_ARRAYBUFFER_HPP_)
#define OMR_OM_ARRAYBUFFER_HPP_

#include <OMR/Infra/HashUtilities.hpp>
#include <OMR/Om/ArrayBufferMap.hpp>

#include <cassert>
#include <type_traits>

namespace OMR {
namespace Om {

/// A managed cell of memory providing general-purpose raw storage.
/// Note that the ArrayBuffer is considered a leaf-object, references stored
/// into the buffer must be explicitly marked.
template <typename T = char>
class ArrayBuffer {
 public:
  union Base {
    Cell cell;
  };

  /// @group Allocating
  /// @{

  /// Allocate a new ArrayBuffer<T>. GC Safepoint. size is the number of
  /// elements, not the size in bytes of the buffer.
  static ArrayBuffer<T>* allocate(Context& cx, std::size_t size);

#if 0
  /// Allocate a new ArrayBuffer<T>. A GC safepoint.
  static ArrayBuffer<T>* reallocate(Context& cx, ArrayBuffer<T>* self,
                                    std::size_t size) {
    return nullptr;
  }
#endif

  /// @}

  ArrayBuffer(ArrayBufferMap* map, std::size_t size);

  Base& base() noexcept { return base_; }

  /// @group Low level data accessors.
  /// @{

  T& get(std::size_t index) noexcept {
    assert(index < size());
    return data_[index];
  }

  const T& get(std::size_t index) const noexcept {
    assert(index < size());
    return data_[index];
  }

  T& set(std::size_t index, const T& value) noexcept {
    return (data_[index] = value);
  }

  /// The array buffer has a generic method for calculating a hash of the data.
  /// In general, it is preferable that the user provides a less-general hashing
  /// function.
  static std::size_t hash(const ArrayBuffer<T>* self) {
    std::size_t hash = self->size_ + 1;
    for (std::size_t i = 0; i < self->size_; i++) {
      hash = Infra::Hash::mix(hash, self->data[i]);
    }
    return hash;
  }

  // Number of elements of T in array.
  std::size_t size() const { return sizeInBytes() / sizeof(T); }

  /// Size of data array in bytes. Not the full size of the object, for that,
  /// see footprint().
  std::size_t sizeInBytes() const { return size_; }

  // Size of this object's allocation. Does not include sub-allocations.
  std::size_t sizeInBytesWithHeader() const {
    return sizeInBytes() + sizeof(ArrayBuffer);
  }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    base_.cell.visit(cx, visitor);
  }

 protected:
  Base base_;
  std::size_t size_;  //< size in bytes of the data array.
  T data_[0];
};

static_assert(std::is_standard_layout<ArrayBuffer<char>>::value,
              "ArrayBuffer must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ARRAYBUFFER_HPP_
