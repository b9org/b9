#ifndef OMR_OM_OBJECT_HPP_
#define OMR_OM_OBJECT_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/Value.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <new>

namespace OMR {
namespace Om {

/// A Cell with dynamically allocated slots.
class Object : public Cell {
 public:
  Object(ObjectMap* map);

  Object(EmptyObjectMap* map);

  Object(Object& other);

  Value* slots();

  const Value* slots() const;

  /// Returns {index, true} on success, or {0, false} on failure.
  /// Note that {0, true} is the first slot in the object.
  std::pair<Index, bool> index(Id id);

  std::pair<Value, bool> get(Context& cx, Id id);

  Value getAt(Index index);

  /// Set the slot that corresponds to the id. If the slot doesn't exist,
  /// allocate the slot and assign it. The result is the address of the slot.
  /// !CAN_GC!
  bool set(Context& cx, Id id, Value value);

  void setAt(Context& cx, Index index, Value value) noexcept;

  void setAt(Context& cx, Index index, Object* value) noexcept;

  void setAt(Context& cx, Index index, std::int32_t value) noexcept;

  /// Allocate a new slot corresponding to the id. The object may not already
  /// have a slot with this Id matching. !CAN_GC!
  Index newSlot(Context& cx, Id id);

 private:
  static constexpr Index MAX_SLOTS = 32;

  Value slots_[MAX_SLOTS];
};

}  // namespace Om
}  // namespace OMR

#endif  // B9_OBJECTS_HPP_
