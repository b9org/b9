#if !defined(OMR_OM_ARRAYBUFFER_INL_HPP_)
#define OMR_OM_ARRAYBUFFER_INL_HPP_

#include <OMR/Om/ArrayBuffer.inl.hpp>

#include <OMR/Om/Allocator.inl.hpp>
#include <OMR/Om/Context.inl.hpp>

namespace OMR {
namespace Om {

template <typename T>
inline ArrayBuffer<T>::ArrayBuffer(ArrayBufferMap* map, std::size_t size)
    : base_{{&map->baseMap()}}, size_(size) {
}

template <typename T>
class ArrayBufferInitializer : public Initializer {
 public:
  explicit ArrayBufferInitializer(std::size_t size) : size_(size) {}

  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto buffer = reinterpret_cast<ArrayBuffer<T>*>(cell);
    auto map = cx.globals().arrayBufferMap();
    new (buffer) ArrayBuffer<T>(map, size_);
    return &buffer->base().cell;
  }

 private:
  std::size_t size_;  //< Size of buffer, not number of elements.
};

template <typename T>
inline ArrayBuffer<T>* ArrayBuffer<T>::allocate(Context& cx,
                                                std::size_t capacity) {
  std::size_t dataSize = capacity * sizeof(T);
  std::size_t allocSize = sizeof(ArrayBuffer) + dataSize;
  ArrayBufferInitializer<T> init(dataSize);
  return BaseAllocator::allocate<ArrayBuffer<T>>(cx, init, allocSize);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ARRAYBUFFER_INL_HPP_
