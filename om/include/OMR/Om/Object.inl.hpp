#if !defined(OMR_OM_OBJECT_INL_HPP_)
#define OMR_OM_OBJECT_INL_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Object.hpp>

#include <StandardWriteBarrier.hpp>

namespace OMR {
namespace Om {

inline Object* Object::allocate(Context& cx, Handle<ObjectMap> map) {
  RootRef<Object> object = nulltptr; // TODO: implement allocation
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

inline void Object::set(Context& cx, Object* self, Index index, Value value) {
  self->slots[index] = value;
}

inline bool Object::index(Context& cx, Object* self, Id id, Index& result) {
  for (auto m = Cell::map(self); Map::kind(m) != MapKind::EMPTY_OBJECT_MAP;) {
    assert(m->kind() == MapKind::SLOT_MAP);
    auto sm = reinterpret_cast<SlotMap*>(m);
    if (sm->id() == id) {
      return {sm->index(), true};
    }
    m = sm->parent();
  }
  return {0, false};
}

inline bool Object::get(Context& cx, Object* self, Id id, Value& result) {
  Index idx;
  auto found = index(cx, self, id, idx);
  if (found) {
    get(cx, self, idx, result);
  }
  return found;
}

inline void Object::get(Context& cx, Object* self, Index index, Value& result) {
  result = self->slots_[index];
}

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

inline void Object::setAt(Context& cx, Index index, Value value) noexcept {
  if (value.isPtr()) {
    // standardWriteBarrier(cx, this, value.getPtr());
  }
  slots_[index] = value;
}

inline void Object::setAt(Context& cx, Index index, Object* value) noexcept {
  // standardWriteBarrier(cx, this, value);
  slots_[index].setPtr(value);
}

inline void Object::setAt(Context& cx, Index index,
                          std::int32_t value) noexcept {
  slots_[index].setInteger(value);
}

/// Allocate a new slot corresponding to the id. The object may not already
/// have a slot with this Id matching. !CAN_GC!
inline Index Object::newSlot(Context& cx, Id id) {
  RootRef<Object> root(cx, this);
  auto m = allocateSlotMap(cx, map(), id);
  root->map(m);
  return m->index();
}

inline bool setSlot(Context& cx, Object* obj, Id slotId, Value value) {
  auto lookup = obj->index(slotId);
  if (std::get<bool>(lookup)) {
    auto index = std::get<Index>(lookup);
    obj->setAt(cx, index, value);
    return false;
  } else {
    RootRef<Object> root(cx, obj);
    auto index = root->newSlot(cx, slotId);
    root->setAt(cx, index, value);
    return true;
  }
}

}  // namespace Om
}  // namespace OMR

#endif  // B9_OBJECTS_INL_HPP_
