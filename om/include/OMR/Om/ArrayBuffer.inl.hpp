#if !defined(OMR_OM_ARRAYBUFFER_INL_HPP_)
#define OMR_OM_ARRAYBUFFER_INL_HPP_

#include <OMR/Om/ArrayBuffer.inl.hpp>

#include <OMR/Om/Allocator.inl.hpp>
#include <OMR/Om/Context.inl.hpp>

namespace OMR {
namespace Om {

inline ArrayBuffer::ArrayBuffer(ArrayBufferMap* map, std::size_t size)
    : baseCell_(&map->baseMap()), size_(size) {}

class ArrayBufferInitializer : public Initializer {
 public:
  explicit ArrayBufferInitializer(std::size_t size) : size_(size) {}

  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto buffer = reinterpret_cast<ArrayBuffer*>(cell);
    auto map = cx.globals().arrayBufferMap();
    new (buffer) ArrayBuffer(map, size_);
    return &buffer->baseCell();
  }

 private:
  std::size_t size_;  //< Size of buffer, not number of elements.
};

inline ArrayBuffer* ArrayBuffer::allocate(Context& cx, std::size_t size) {
  ArrayBufferInitializer init(size);
  return BaseAllocator::allocate<ArrayBuffer>(cx, init, calcAllocSize(size));
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ARRAYBUFFER_INL_HPP_
