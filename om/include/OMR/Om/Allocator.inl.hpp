#ifndef OMR_OM_ALLOCATOR_INL_HPP_
#define OMR_OM_ALLOCATOR_INL_HPP_

#include <OMR/Om/Allocator.hpp>
#include <OMR/Om/Object.inl.hpp>

namespace OMR {
namespace Om {

struct RawAllocator {};

/// Common allocation path. Allocate an instance of T. Throws std::bad_alloc.
template <typename T, typename Initializer>
inline T* allocate(Context& cx, Initializer& init, std::size_t size) {
  Allocation allocation(cx, init, size);
  auto p = (T*)OMR_GC_AllocateObject(cx.omrVmThread(), &allocation);

  if (p == nullptr) {
    throw std::bad_alloc();
  }

  if (DEBUG_SLOW) {
    RootRef<T> root(cx, p);
    OMR_GC_SystemCollect(cx.omrVmThread(), 0);
    p = root.get();
  }

  return p;
}

inline MetaMap* allocateMetaMap(Context& cx) {
  MetaMapInitializer init;
  return allocate<MetaMap>(cx, init);
}

inline EmptyObjectMap* allocateEmptyObjectMap(Context& cx) {
  EmptyObjectMapInitializer init;
  return allocate<EmptyObjectMap>(cx, init);
}

inline SlotMap* allocateSlotMap(Context& cx, Map* parent, Id slotId) {
  SlotMapInitializer init(cx, parent, slotId);
  return allocate<SlotMap>(cx, init);
}

inline Object* allocateEmptyObject(Context& cx) {
  EmptyObjectInitializer init;
  return allocate<Object>(cx, init);
}

inline Object* allocateObject(Context& cx, Object* base) {
  ObjectInitializer init(cx, base);
  return allocate<Object>(cx, init);
}

}  // namespace Om
}  // namespace OMR
#endif  // OMR_OM_ALLOCATOR_INL_HPP_