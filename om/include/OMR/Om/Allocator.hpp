#ifndef OMR_OM_ALLOCATOR_HPP_
#define OMR_OM_ALLOCATOR_HPP_

#include <OMR/Om/Allocation.hpp>
#include <OMR/Om/Context.hpp>
#include <OMR/Om/Object.hpp>
#include <OMR/Om/Runtime.hpp>

#include <omrgc.h>

namespace OMR {
namespace Om {

constexpr bool DEBUG_SLOW = true;

/// Common allocation path. Allocate an instance of T. Throws std::bad_alloc.
template <typename T, typename Initializer>
inline T* allocate(Context& cx, Initializer& init,
                   std::size_t size = sizeof(T));

inline MetaMap* allocateMetaMap(Context& cx);

inline EmptyObjectMap* allocateEmptyObjectMap(Context& cx);

inline SlotMap* allocateSlotMap(Context& cx, Map* parent, Id slotId);

inline Object* allocateEmptyObject(Context& cx);

inline Object* allocateObject(Context& cx, Object* base);

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ALLOCATOR_HPP_
