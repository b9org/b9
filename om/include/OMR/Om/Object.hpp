#ifndef OMR_OM_OBJECT_HPP_
#define OMR_OM_OBJECT_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/Value.hpp>
#include <OMR/Om/SlotMap.hpp>
#include <OMR/Om/MemVector.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <new>
#include <type_traits>

namespace OMR {
namespace Om {

/// A Cell with dynamically allocated slots.
struct Object {
  union Base {
    Cell cell;
  };

  static Object* allocate(Context& cx, Handle<ObjectMap> map);

  static Object* clone(Context& cx, Handle<Object> base);

  static void construct(Context& cx, Handle<Object> self, Handle<ObjectMap> map);

  static bool get(Context& cx, const Object* self, Id id, Value& result);

  static void get(Context& cx, const Object* self, Index index, Value& result);

  static void set(Context& cx, Object* self, Index index, Value value) noexcept;

  static bool index(Context& cx, const Object* self, Id id, Index& result);

  /// Set the slot that corresponds to the id. If the slot doesn't exist,
  /// allocate the slot and assign it. The result is the address of the slot.
  /// !CAN_GC!
  static bool set(Context& cx, Handle<Object> self, Id id, Value value);

  /// Allocate a new slot corresponding to the id. The object may not already
  /// have a slot with this Id matching. !CAN_GC!
  static Index newSlot(Context& cx, Handle<Object> self, Id id);

  static ObjectMap* objectMap(const Object* object) {
    return reinterpret_cast<ObjectMap*>(object->base.cell.map());
}

  static constexpr Index MAX_SLOTS = 32;

  Base base;
  MemVector<Value> dynamicSlots;
  std::size_t fixedSlotCount;
  Value fixedSlots[0];
};

static_assert(std::is_standard_layout<Object>::value, "Object must be a StandardLayoutType.");

}  // namespace Om
}  // namespace OMR

#endif  // B9_OBJECTS_HPP_
