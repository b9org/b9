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

inline SlotMap* allocateSlotMap(Context& cx, Handle<ObjectMap> parent,
                                const SlotDescriptor& desc) {
  SlotMapInitializer init(parent, desc);
  return allocate<SlotMap>(cx, init);
}

/// Allocate empty object.
inline Object* allocateObject(Context& cx, Handle<EmptyObjectMap> map) {
  ObjectInitializer init;
  init.map_ = map.reinterpret<ObjectMap>();
  return allocate<Object>(cx, init);
}

/// Allocate object with corresponding slot map;
inline Object* allocateObject(Context& cx, Handle<SlotMap> map) {
  ObjectInitializer init;
  init.map_ = map.reinterpret<ObjectMap>();
  return allocate<Object>(cx, init);
}

inline Object* allocateEmptyObject(Context& cx) {
  Object* object = nullptr;

  RootRef<EmptyObjectMap> map(cx, allocateEmptyObjectMap(cx));
  if (map.get() != nullptr) {
    object = allocateObject(cx, map);
  }

  return object;
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ALLOCATOR_INL_HPP_
