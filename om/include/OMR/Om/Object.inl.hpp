#if !defined(OMR_OM_OBJECT_INL_HPP_)
#define OMR_OM_OBJECT_INL_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Object.hpp>

#include <StandardWriteBarrier.hpp>

namespace OMR {
namespace Om {

#if 0  //////////////////////////////////////////

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

#endif  ////////////////////////////////////////////

inline bool Object::index(Context& cx, const Object* self,
                          const SlotDescriptor& desc, Index& result) {
  const ObjectMap* objectMap = self->map();

  while (objectMap->kind() != Map::Kind::EMPTY_OBJECT_MAP) {
    assert(objectMap->kind() == Map::Kind::SLOT_MAP);
    auto slotMap = reinterpret_cast<const SlotMap*>(objectMap);
    if (slotMap->desc_ == desc) {
      result = slotMap->index();
      return true;
    }
    objectMap = slotMap->parent();
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

inline void Object::get(Context& cx, const Object* self, Index index,
                        Value& result) {
  result = self->get(index);
}

#if 0
/// Set the slot that corresponds to the id. If the slot doesn't exist,
/// allocate the slot and assign it. The result is the address of the slot.
/// !CAN_GC!
inline bool Object::set(Context& cx, Handle<Object> object, const SlotDescriptor& desc,
                        Value value) {
  std::size_t hash = desc.hash();

  Index idx = -1;
  bool found = index(cx, object, desc, idx);

  if (!found) {
    auto map = transition(cx, handle, desc, hash);  // TODO: Write barrier on transition
    if (map == nullptr) {
      return false;
    }
    idx = map->index();
    object = root.get();
  }

  set(cx, object, idx, value);  // TODO: Write barrier in set
  return true;
}
#endif

inline void Object::set(Context& cx, Object* self, Index index,
                        Value value) noexcept {
  if (value.isPtr()) {
    // standardWriteBarrier(cx, this, value.getPtr());
  }
  self->fixedSlots_[index] = value;
}

inline SlotMap* Object::lookUpTransition(Context& cx,
                                         const SlotDescriptor& desc,
                                         std::size_t hash) {
  return map()->lookUpTransition(cx, desc, hash);
}

#if 0
inline bool Object::takeExistingTransition(Context& cx, const ObjectDescription& desc, Index& result) {
  auto hash = desc.hash();
  return takeExistingTransition(desc, hash, result);
}
#endif

inline SlotMap* Object::takeExistingTransition(Context& cx,
                                               const SlotDescriptor& desc,
                                               std::size_t hash) {
  SlotMap* derivation = lookUpTransition(cx, desc, hash);
  if (derivation != nullptr) {
    map(&derivation->baseObjectMap());
    // TODO: Write barrier here?
  }
  return derivation;
}

#if 0
inline Index Object::takeNewTransistion(Context& cx, Handle<Object> self, const SlotDescriptor& desc) {
  auto hash = desc.hash();
  return takeNewTransistion(cx, self, desc, hash);
}
#endif

inline SlotMap* Object::takeNewTransition(Context& cx, Handle<Object> object,
                                          const SlotDescriptor& desc,
                                          std::size_t hash) {
  RootRef<ObjectMap> base(cx, object->map());
  SlotMap* derivation = ObjectMap::derive(cx, base, desc, hash);
  object->map(&derivation->baseObjectMap());
  return derivation;
  // TODO: Write barrier on objects taking a new transition
}

inline SlotMap* Object::transition(Context& cx, Handle<Object> object,
                                   const SlotDescriptor& desc,
                                   std::size_t hash) {
  SlotMap* derivation = object->takeExistingTransition(cx, desc, hash);
  if (derivation == nullptr) {
    derivation = takeNewTransition(cx, object, desc, hash);
  }
  return derivation;
  // TODO: Write barrier on transitioning objects.
}

}  // namespace Om
}  // namespace OMR

#endif  // B9_OBJECTS_INL_HPP_
