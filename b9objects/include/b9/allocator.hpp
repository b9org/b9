#if !defined(B9_ALLOCATOR_HPP_)
#define B9_ALLOCATOR_HPP_

#include <omrgc.h>
#include <ObjectAllocationModel.hpp>
#include <b9/context.hpp>
#include <b9/objects.hpp>
#include <b9/runtime.hpp>

namespace b9 {

constexpr bool B9_DEBUG_SLOW = true;

/// Common allocation path. Allocate an instance of T. Throws std::bad_alloc.
template <typename T, typename Initializer>
inline T* allocate(Context& cx, Initializer& init,
                   std::size_t size = sizeof(T));

inline MetaMap* allocateMetaMap(Context& cx);

inline EmptyObjectMap* allocateEmptyObjectMap(Context& cx);

inline SlotMap* allocateSlotMap(Context& cx, Map* parent, Id slotId);

inline Object* allocateEmptyObject(Context& cx);

inline Object* allocateObject(Context& cx, Object* base);

}  // namespace b9

#endif  // B9_ALLOCATOR_HPP_
