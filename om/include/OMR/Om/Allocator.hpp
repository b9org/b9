#ifndef OMR_OM_ALLOCATOR_HPP_
#define OMR_OM_ALLOCATOR_HPP_

#include <OMR/Om/Allocation.hpp>
#include <OMR/Om/ArrayBuffer.hpp>
#include <OMR/Om/Context.hpp>
#include <OMR/Om/Object.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/Runtime.hpp>
#include <OMR/Om/SlotMap.hpp>
// #include <OMR/Om/

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

inline Object* allocateObject(Context& cx, Handle<EmptyObjectMap> map);

inline Object* allocateObject(Context& cx, Handle<SlotMap> map);

inline Object* allocateEmptyObject(Context& cx);

template <typename T>
inline ArrayBuffer<T>* allocateArrayBuffer(Context& cx, std::size_t size);

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ALLOCATOR_HPP_
