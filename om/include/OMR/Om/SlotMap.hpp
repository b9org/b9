#if !defined(OMR_OM_SLOTMAP_HPP_)
#define OMR_OM_SLOTMAP_HPP_

#include <OMR/Om/Map.hpp>
#include <OMR/Om/MemTransitionSet.hpp>
#include <OMR/Om/MetaMap.hpp>
#include <OMR/Om/SlotDescriptor.hpp>

#include <cassert>

namespace OMR {
namespace Om {

using Index = std::uint8_t;

/// The SlotMap is an ObjectMap that describes a slot (aka field or property) of
/// an object.
struct SlotMap {
  union Base {
    ObjectMap objectMap;
    Map map;
    Cell cell;
  };

  static SlotMap* allocate(Context& cx);

  static void construct(Context& cx, SlotMap* self, const ObjectMap* parent, const SlotDescriptor& desc);

  /// Derive a new slot map. Do not add the slot map to the parent's transition table.
  static SlotMap* derive(Context& cx, Handle<ObjectMap> parent, const SlotDescriptor& desc);

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  Map& baseMap() { return base().map; }
  
  const Map& baseMap() const { return base().map; }

  ObjectMap& baseObjectMap() { return base().objectMap; }

  const ObjectMap& baseObjectMap() const { return base().objectMap; }

  Map::Kind kind() const { return baseMap().kind(); }

  Base base_;
  ObjectMap* parent;
  SlotDescriptor desc;
  Index index;
};

static_assert(std::is_standard_layout<SlotMap>::value,
              "SlotMap must be a StandardLayoutType.");


inline SlotMap* SlotMap::allocate(Context& cx) {
  return nullptr;
}

inline SlotMap* SlotMap::derive(Context& cx, Handle<ObjectMap> parent, const SlotDescriptor& desc) {
  auto child = SlotMap::allocate(cx);
  ObjectMap::construct(cx, &child->baseObjectMap(), cx.globals().metaMap, Map::Kind::SLOT_MAP);
  if (parent->kind() == Map::Kind::SLOT_MAP) {
    auto p = reinterpret_cast<SlotMap*>(parent.ptr());
    child->index = p->index + 1;
  } else {
    child->index = 0;
  }
  child->desc = desc;
  // TODO: post write barrier on the child
  return child;
}

#if 0

inline void construct(Context& cx, SlotMap* self, SlotMap* parent, const SlotDescriptor& desc) {
  ObjectMap::construct(cx, reinterpret_cast<ObjectMap>(self), )
}


  static void construct(Context& cx, SlotMap* self, SlotMap* parent,
                        const SlotDescriptor& desc) {
    ObjectMap::construct(cx, self, Map::metaMap(parent->objectMap.map),
                         Map::Kind::SLOT_MAP);
    self->index = parent->index += 1;
    self->parent = parent;
    self->desc = desc;
  }

  static void construct(Context& cx, SlotMap* self, EmptyObjectMap* parent,
                        const SlotDescriptor& desc) {
    ObjectMap::construct(cx, self, Map::metaMap(cx, parent),
                         Map::Kind::SLOT_MAP);
    self->index = 0;
    self->parent = parent;
    self->desc = desc;
  }

  static void construct(Context& cx, SlotMap* self, ObjectMap* parent,
                        const SlotDescriptor& desc) {

    if (parent->kind == Map::Kind::EMPTY_OBJECT_MAP) {
      construct(cx, self, static_cast<EmptyObjectMap*>(parent), desc);
    } else {
      assert(parent->kind == Map::Kind::SLOT_MAP);
      construct(cx, self, static_cast<SlotMap*>(parent), desc);
    }
  }

#endif

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_SLOTMAP_HPP_
