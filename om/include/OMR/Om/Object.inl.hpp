#if !defined(OMR_OM_OBJECT_INL_HPP_)
#define OMR_OM_OBJECT_INL_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Object.hpp>

#include <StandardWriteBarrier.hpp>

namespace OMR {
namespace Om {

#if 0 //////////////////////////////////////////

inline Object* Object::allocate(Context& cx, Handle<ObjectMap> map) {
  RootRef<Object> object = nullptr; // TODO: implement allocation
  construct(cx, object.reinterpret<Cell>(), map);
  return object.ptr();
}

inline void Object::construct(Context& cx, Handle<Object> self,
                              Handle<ObjectMap> map) {
  Cell::construct(cx, self, map);
  self->fixedSlots = fixedSlots;
  memset(slots_, 0, MAX_SLOTS * sizeof(Value));
  MemVector<Value>::construct(cx, {self, &Object::dynamicSlots}, 0);
}

inline void Object::clone(Context& xc, Handle<Object> base) {
  allocate(cx, base->map);
}

#endif ////////////////////////////////////////////

inline bool Object::index(Context& cx, const Object* self, Id id, Index& result) {
  const ObjectMap* objectMap = self->map();
  while (objectMap->kind() != Map::Kind::EMPTY_OBJECT_MAP) {
    assert(objectMap->kind() == Map::Kind::SLOT_MAP);
    auto slotMap = reinterpret_cast<const SlotMap*>(objectMap);
    if (slotMap->desc.id() == id) {
      result = slotMap->index;
      return true;
    }
    objectMap = slotMap->parent;
  }
  result = -1;
  return false;
}

inline bool Object::get(Context& cx, const Object* self, Id id, Value& result) {
  Index index = -1;
  auto found = Object::index(cx, self, id, index);
  if (found) {
    Object::get(cx, self, index, result);
  }
  return found;
}

inline void Object::get(Context& cx, const Object* self, Index index, Value& result) {
  result = self->fixedSlots[index];
}

#if 0
/// Set the slot that corresponds to the id. If the slot doesn't exist,
/// allocate the slot and assign it. The result is the address of the slot.
/// !CAN_GC!
inline bool Object::set(Context& cx, Id id, Value value) {
  auto lookup = index(id);
  if (std::get<bool>(lookup)) {
    // TODO: WB.
    auto index = std::get<Index>(lookup);
    set(cx, index, value);
    return true;
  }
  return false;
}

inline bool Object::set(Context& cx, Id id, Value value) {}
#endif

inline void Object::set(Context& cx, Object* self, Index index, Value value) noexcept {
  if (value.isPtr()) {
    // standardWriteBarrier(cx, this, value.getPtr());
  }
  self->fixedSlots[index] = value;
}

/// Allocate a new slot corresponding to the id. The object may not already
/// have a slot with this Id matching. !CAN_GC!
inline Index Object::newSlot(Context& cx, Handle<Object> self, Id id) {
#if 0
  RootRef<ObjectMap> baseMap(Object::map(self));
  SlotMap* newMap = SlotMap::derive(cx, baseMap, id);
  auto m = allocateSlotMap(cx, map(), id);
  root->map(m);
  return m->index();
#endif
  assert(0);
  return -1;
}

}  // namespace Om
}  // namespace OMR

#endif  // B9_OBJECTS_INL_HPP_
