#ifndef OMR_OM_MAP_HPP_
#define OMR_OM_MAP_HPP_

#include <OMR/Om/Cell.hpp>

namespace OMR {
namespace Om {

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
};

}  // namespace Om
}  // namespace OMR

namespace std {

template <>
struct hash<OMR::Om::Map> {
  std::size_t operator()(const OMR::Om::Map& map) const {
    return 0 /* TODO: map.hash() */;
  }
};

}  // namespace std

#endif  // OMR_OM_MAP_HPP_
