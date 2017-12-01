#if !defined(B9_OBJECTS_INL_HPP_)
#define B9_OBJECTS_INL_HPP_

#include <b9/context.hpp>
#include <b9/objects.hpp>
// #include <b9/allocator.hpp>

/// !CAN_GC!
inline void* operator new(std::size_t size, b9::Context& cx) {
  std::cerr << "> allocating: " << size << "B\n";
  return malloc(size);  // TODO
}

namespace b9 {

inline Object::Object(ObjectMap* map) : Cell(map) {
  memset(slots_, 0, MAX_SLOTS * sizeof(Value));
}

inline Object::Object(EmptyObjectMap* map) : Cell(map) {
  memset(slots_, 0, MAX_SLOTS * sizeof(Value));
}

inline Object::Object(Object& other) : Cell(other.map()) {
  memcpy(slots_, other.slots_, MAX_SLOTS * sizeof(Value));
}

inline Value* Object::slots() { return slots_; }

inline const Value* Object::slots() const { return slots_; }

/// Returns {index, true} on success, or {0, false} on failure.
/// Note that {0, true} is the first slot in the object.
inline std::pair<Index, bool> Object::index(Id id) {
  for (auto m = map(); m->kind() != MapKind::EMPTY_OBJECT_MAP;) {
    // assert(m->kind() == MapKind::OBJECT_MAP);
    auto om = (ObjectMap*)m;
    if (om->id() == id) {
      return {om->index(), true};
    }
    m = om->parent();
  }
  return std::make_pair(0, false);
}

inline std::pair<Value, bool> Object::get(Context& cx, Id id) {
  auto lookup = index(id);
  if (lookup.second) {
    Value value = slots_[lookup.first];
    return std::make_pair(value, true);
  } else {
    return std::make_pair(0, false);
  }
}

/// Set the slot that corresponds to the id. If the slot doesn't exist,
/// allocate the slot and assign it. The result is the address of the slot.
/// !CAN_GC!
inline bool Object::set(Context& cx, Id id, Value value) {
  auto lookup = index(id);
  if (std::get<bool>(lookup)) {
    auto index = std::get<Index>(lookup);
    slots_[index] = value;
    return true;
  }
  return false;
}

inline void Object::setAt(Context& cx, Index index, Value value) {
  slots_[index] = value;
}

/// Allocate a new slot corresponding to the id. The object may not already
/// have a slot with this Id matching. !CAN_GC!
inline Index Object::newSlot(Context& cx, Id id) {
  ObjectMap* m;
  switch (map()->kind()) {
    case MapKind::EMPTY_OBJECT_MAP:
      m = new (cx) ObjectMap((EmptyObjectMap*)map(), id);
      break;
    case MapKind::OBJECT_MAP:
      m = new (cx) ObjectMap((ObjectMap*)map(), id);
      break;
    default:
      throw std::runtime_error(
          "An object has a map that is neither an ObjectMap nor "
          "EmptyObjectMap");
      m = nullptr;
      break;
  }
  map(m);
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

}  // namespace b9

#endif  // B9_OBJECTS_INL_HPP_
