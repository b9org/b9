#if !defined(B9_OBJECTS_HPP_)
#define B9_OBJECTS_HPP_

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

///
/// The Cell header
///

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

class MapMap;

enum class MapKind { OBJECT_MAP, MAP_MAP, EMPTY_OBJECT_MAP };

class Map : public Cell {
 public:
  Map(MapMap* map, MapKind kind) noexcept;

  MapMap* mapMap() const;

  MapKind kind() const { return kind_; }

 private:
  MapKind kind_;
};

class MapMap : public Map {
 public:
  MapMap() : Map(this, MapKind::MAP_MAP) {}
};

inline Map::Map(MapMap* map, MapKind kind) noexcept : Cell(map), kind_(kind) {}

inline MapMap* Map::mapMap() const { return (MapMap*)map(); }

class EmptyObjectMap : public Map {
 public:
  EmptyObjectMap(MapMap* map) : Map(map, MapKind::EMPTY_OBJECT_MAP) {}
};

using Index = std::uint8_t;

class ObjectMap : public Map {
 public:
  ObjectMap(EmptyObjectMap* parent, Id id)
      : Map(parent->mapMap(), MapKind::OBJECT_MAP),
        parent_(parent),
        id_(id),
        index_(0) {}

  ObjectMap(ObjectMap* parent, Id id)
      : Map(parent->mapMap(), MapKind::OBJECT_MAP),
        parent_(parent),
        id_(id),
        index_(parent->index() + 1) {}

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
};

using Value = std::int32_t;

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
    return new (cx->allocator()) ObjectMap(cx->mapMap(), parent);
  }

  newObject(Context& cx, ObjectMap* map) { return new (cx) Object(map); }

  newObject(Context& cx, Object* object) { return new (cx) Object(*object); }
#endif

}  // namespace b9

#endif  // B9_OBJECTS_HPP_
