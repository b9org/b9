#if !defined(B9_ALLOCATOR_HPP_)
#define B9_ALLOCATOR_HPP_

#include <omrgc.h>
#include <ObjectAllocationModel.hpp>
#include <b9/context.hpp>
#include <b9/objects.hpp>
#include <b9/objects.inl.hpp>
#include <b9/runtime.hpp>

namespace b9 {

struct RawAllocator {};

struct Allocator {
#if 0
  template <typename T, typename... Args>
  T* allocate(Context& cx, Args&&... args) {
    auto p = rawAllocator.allocate(sizeof(T));

    /// Some sneaky notes: The constructor for T should not sub-allocate. It
    /// must do the minimum initialization to make the object walkable, in the
    /// case of a concurrent GC scan.
    new (p)(std::forward<Args>(args)...);

    cx->saveStack().push(p);
    pay_tax(cx);
    cx->saveStack.pop();
  }
#endif
};

template <typename T, typename Allocation>
T* allocate(Context& cx, Allocation& allocation) {
  auto p = (T*)OMR_GC_AllocateObject(cx.omrVmThread(), &allocation);
  return p;
}

inline MapMap* allocateMapMap(Context& cx) {
  MapMapInitializer init;
  Allocation allocation(cx, init, sizeof(MapMap));
  return allocate<MapMap>(cx, allocation);
}

inline EmptyObjectMap* allocateEmptyObjectMap(Context& cx) {
  MapInitializer init;
  init.mapMap = cx.globals().mapMap;
  init.kind = MapKind::EMPTY_OBJECT_MAP;
  Allocation allocation(cx, init, sizeof(EmptyObjectMap));
  return allocate<EmptyObjectMap>(cx, allocation);
}

inline Object* allocateObject(Context& cx) {
  EmptyObjectInitializer init;
  init.map = cx.globals().emptyObjectMap;
  Allocation allocation(cx, init, sizeof(Object));
  return allocate<Object>(cx, allocation);
}

}  // namespace b9

#endif  // B9_ALLOCATOR_HPP_
