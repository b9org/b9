#if !defined(B9_OBJECTS_HPP_)
#define B9_OBJECTS_HPP_

#include <b9/value.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <new>

namespace b9 {

class Map;
class Context;
class MemoryManager;

/// A numeric unique id. ID's can be used to identify anything.
/// Any heap allocated thing, or 32-bit number, or string symbol, can be
/// converted to an ID. The ID used for a number of lookups. In particular, IDs
/// are used for object-field lookup.
using Id = std::uint32_t;

using RawCellHeader = std::uintptr_t;

class CellHeader {
 public:
  CellHeader(Map* m = nullptr) : value_(RawCellHeader(m) << MAP_SHIFT) {}

  void map(Map* m) {
    value_ = (RawCellHeader(m) << MAP_SHIFT) | (value_ & FLAGS_MASK);
  }

  Map* map() const { return (Map*)(value_ >> MAP_SHIFT); }

  void set(Map* m, std::uint8_t flags) {
    value_ = (RawCellHeader(m) << MAP_SHIFT) | (value_ & FLAGS_MASK);
  }

 private:
  static constexpr RawCellHeader FLAGS_MASK = 0xFF;
  static constexpr std::size_t MAP_SHIFT = 8;

  RawCellHeader value_;
};

/// A managed blob of memory. All Cells have a one slot header.
class Cell {
 public:
  Cell(Map* map) : header_(map) {}

  Map* map() const { return header_.map(); }

  Cell& map(Map* m) {
    header_.map(m);
    return *this;
  }

 private:
  CellHeader header_;
};

class MetaMap;

enum class MapKind { SLOT_MAP, META_MAP, EMPTY_OBJECT_MAP };

/// A description of a cell's layout, or shape. The Map is akin to a java class,
/// except that Maps are typically very small. Every Cell has a Map. Maps may be
/// shared by Cells. The MapKind can be examined to tell what kind of thing the
/// cell is.
class Map : public Cell {
 public:
  Map(MetaMap* map, MapKind kind) noexcept;

  MetaMap* metaMap() const;

  MapKind kind() const { return kind_; }

 private:
  MapKind kind_;
};

/// A map that describes the shape of other Maps. The MetaMap is self
/// descriptive, IE metaMap->map() == metaMap. The MetaMap is a heap-wide
/// singleton.
class MetaMap : public Map {
 public:
  MetaMap() : Map(this, MapKind::META_MAP) {}
};

inline Map::Map(MetaMap* map, MapKind kind) noexcept : Cell(map), kind_(kind) {}

inline MetaMap* Map::metaMap() const { return (MetaMap*)map(); }

/// A map that describes the layout of an object. ObjectMaps are either the
/// EmptyObjectMap, or a SlotMap.
class ObjectMap : public Map {
 protected:
  /// ObjectMap uses the same constructors as MetaMap, but those constructors
  /// are only callable from subclasses. ObjectMaps are not directly
  /// constructible.
  using Map::Map;
};

/// A special ObjectMap that indicates an object has no slots. Every empty
/// object has an empty object map. The EmptyObjectMap is the root of all
/// ObjectMap lineages. The EmptyObjectMap is a heap-wide-singleton.
class EmptyObjectMap : public ObjectMap {
 public:
  EmptyObjectMap(MetaMap* map) : ObjectMap(map, MapKind::EMPTY_OBJECT_MAP) {}
};

using Index = std::uint8_t;

/// The SlotMap is an ObjectMap that describes a slot (aka field or property) of
/// an object.
class SlotMap : public ObjectMap {
 public:
  SlotMap(EmptyObjectMap* parent, Id id)
      : ObjectMap(parent->metaMap(), MapKind::SLOT_MAP),
        parent_(parent),
        id_(id),
        index_(0) {}

  SlotMap(SlotMap* parent, Id id)
      : ObjectMap(parent->metaMap(), MapKind::SLOT_MAP),
        parent_(parent),
        id_(id),
        index_(parent->index() + 1) {}

  SlotMap(ObjectMap* parent, Id id)
      : ObjectMap(parent->metaMap(), MapKind::SLOT_MAP),
        parent_(parent),
        id_(id),
        index_(0) {
    if (parent_->kind() == MapKind::EMPTY_OBJECT_MAP) {
      index_ = 0;
    } else {
      assert(parent_->kind() == MapKind::SLOT_MAP);
      index_ = reinterpret_cast<SlotMap*>(parent_)->index() + 1;
    }
  }

  constexpr Map* parent() const noexcept { return parent_; }

  constexpr Id id() const noexcept { return id_; }

  Map& id(Id id) noexcept {
    id_ = id;
    return *this;
  }

  constexpr Index index() const noexcept { return index_; }

  Map& index(Index index) noexcept {
    index_ = index;
    return *this;
  }

 private:
  Map* parent_;
  // MapTable* children_;
  Id id_;
  Index index_;
};  // namespace b9

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

  /// Set the slot that corresponds to the id. If the slot doesn't exist,
  /// allocate the slot and assign it. The result is the address of the slot.
  /// !CAN_GC!
  bool set(Context& cx, Id id, Value value);

  void setAt(Context& cx, Index index, Value value);

  /// Allocate a new slot corresponding to the id. The object may not already
  /// have a slot with this Id matching. !CAN_GC!
  Index newSlot(Context& cx, Id id);

 private:
  static constexpr Index MAX_SLOTS = 32;

  Value slots_[MAX_SLOTS];
};

#if 0
bool setSlot(Context& cx, Object* obj, Id slotId, Value value) {
  auto lookup = obj->index(slotId);
  if(std::get<bool>(lookup)) {
    auto index = std::get<Index>(lookup);
    obj->setAt(cx, index, value);
  } else {
    RootRef root(cx, obj);
    auto index = root->newSlot(slotId);
    root.ptr->setAt(cx, index, value);
  }
}
#endif

//
// ID Generation and Mapping
//

class IdGenerator {
 public:
  Id newId() { return nextId_++; }

 private:
  Id nextId_ = 0;
};

//
// Misc Object allocators
//

#if 0
  newObjectMap(Context& cx, ObjectMap* parent = nullptr) {
    return new (cx->allocator()) ObjectMap(cx->metaMap(), parent);
  }

  newObject(Context& cx, ObjectMap* map) { return new (cx) Object(map); }

  newObject(Context& cx, Object* object) { return new (cx) Object(*object); }
#endif

}  // namespace b9

#endif  // B9_OBJECTS_HPP_
