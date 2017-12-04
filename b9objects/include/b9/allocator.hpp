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

constexpr bool B9_DEBUG_SLOW = true;

/// Common allocation path. Allocate an instance of T. Throws std::bad_alloc.
template <typename T, typename Initializer>
inline T* allocate(Context& cx, Initializer& init, std::size_t size = sizeof(T)) {

  Allocation allocation(cx, init, size);
  auto p = (T*)OMR_GC_AllocateObject(cx.omrVmThread(), &allocation);

  if (p == nullptr) {
    throw std::bad_alloc();
  }

  if (B9_DEBUG_SLOW) {
    RootRef<T> root(cx, p);
    OMR_GC_SystemCollect(cx.omrVmThread(), 0);
    p = root.ptr();
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

}  // namespace b9

#endif  // B9_ALLOCATOR_HPP_
