#if !defined(B9_OBJECTS_HPP_)
#define B9_OBJECTS_HPP_

#include <cstddef>
#include <cstdint>

namespace b9 {

using Id = std::uint32_t;

class IdGenerator {
 public:
  Id newId() { return nextId_++; }

 private:
  Id nextId_ = 0;
};

class Map;

class Cell {
 public:
  Cell(Map* map) : map_(map) {}

  Map* map() const { return map_; }

 private:
  Map* map_;
};

class MapMap;

enum class MapKind { OBJECT_MAP, MAP_MAP, EMPTY_OBJECT_MAP };

struct Map : public Cell {
 public:
  Map(MapMap* map, MapKind kind) noexcept;

  MapMap* mapMap() const;

 private:
  MapKind kind_;
};

class MapMap : public Map {
 public:
  MapMap() : Map(this, MapKind::MAP_MAP) {}
};

inline Map::Map(MapMap* map, MapKind kind) noexcept : Cell(map), kind_(kind) {}

MapMap* Map::mapMap() const { return (MapMap*)map(); }

class EmptyObjectMap : public Map {
 public:
  EmptyObjectMap(MapMap* map) : Map(map, MapKind::EMPTY_OBJECT_MAP) {}
};

using MapIndex = std::uint8_t;

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

  constexpr Id id() const noexcept { return id_; }

  Map& id(Id id) noexcept {
    id_ = id;
    return *this;
  }

  constexpr MapIndex index() const noexcept { return index_; }

  Map& index(MapIndex index) noexcept {
    index_ = index;
    return *this;
  }

 private:
  Map* parent_;
  // MapTable* children_;
  Id id_;
  MapIndex index_;
};

using Slot = std::uintptr_t;

class Object : public Cell {
 public:
  Object(ObjectMap* map) : map_(map) {
    memset(slots, 0, MAX_SLOTS * sizeof(Slot));
  }

  Object(Object& other) : map_(other.map_) {
    memcpy(slots_, other.slots_, MAX_SLOTS * sizeof(Slot));
  }

  Slot* slots() { return slots_; }

  Slot const* slots() const { return slots_; }

  Slot slot(Id id) {
    for (Map* m = map_; m != nullptr; m = m->parent()) {
      if (m.id() == id) {
        return slots_[m.index()];
      }
    }
    throw std::runtime_error{std::string{"failed to find slot: "} + atoi(id)};
  }

  Slot set(Context& cx, Id id, Slot value) {
    for (Map* m = map_; m != cx->emptyObjectMap(); m = m->parent()) {
      if (m.id() == id) {
        return slots_[m.index()] = value;
      }
    }
    else {
      auto index = newSlot(id);
      slots_[index] = value;
    }
    throw std::runtime_error{std::string{"failed to find slot: "} + atoi(id)};
  }

  std::size_t newSlot(Context& cx, Id id) {
    switch (map_->kind()) {
      case MapKind::EMPTY_OBJECT_MAP:
        map_ = new (cx) ObjectMap((EmptyObjectMap*)map_, id);
        return map_.index();
      case MapKind::OBJECT_MAP:
        map_ = new (cx) ObjectMap((ObjectMap*)map_, id);
        return map_.index();
    }
    return map_.index();
  }

 private:
  static constexpr std::size_t MAX_SLOTS = 32;

  Slot slots_[MAX_SLOTS];
};

newObjectMap(Context& cx, ObjectMap* parent = nullptr) {
  return new (cx->allocator()) ObjectMap(cx->mapMap(), parent);
}

newObject(Context& cx, ObjectMap* map) { return new (cx) Object(map); }

newObject(Context& cx, Object* object) { return new (cx) Object(*object); }

struct Allocator {};

struct Context {
 public:
  Allocator& allocator() { return allocator_; }

 private:
  Allocator allocator_;
  MapMap* mapMap_;
  EmptyObjectMap* emptyObjectMap_;
};

void* operator new(Context& cx, std::size_t size) {
  return OMR_GC_Allocate(cx, size);
}

class SymbolTable {
 public:
  Id lookup(std::string& string) {
    auto it = lookupTable_.find(string);
    if (it != lookupTable.end()) {
      return it->second;
    } else {
      auto id = IdGenerator.newId();
      lookupTable.insert({string, id});
      return id;
    }
  }

 private:
  IdGenerator idGenerator_;
  std::map<std::string, Id> lookupTable_;
};

getSizeOfObject()
}  // namespace b9

#endif  // B9_OBJECTS_HPP_
