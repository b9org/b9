#if !defined(OMR_OM_OBJECTMAP_HPP_)
#define OMR_OM_OBJECTMAP_HPP_

// #include <OMR/Om/Context.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/MemTransitionSet.hpp>
#include <OMR/Om/SlotDescriptor.hpp>

namespace OMR {
namespace Om {

class Context;

/// A map that describes the layout of an object. ObjectMaps are either the
/// EmptyObjectMap, or a SlotMap.
struct ObjectMap {
  union Base {
    Map map;
    Cell cell;
  };

  static void construct(Context& cx, ObjectMap* self, MetaMap* meta,
                        Map::Kind kind);

  static SlotMap* transition(Context& cx, const SlotDescriptor& desc);

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Map& baseMap() { return base().map; }

  const Map& baseMap() const { return base().map; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  Map::Kind kind() const { return baseMap().kind(); }

  Base base_;
  MemTransitionSet transitions_;
};

static_assert(std::is_standard_layout<ObjectMap>::value,
              "ObjectMap must be a StandardLayoutType");

inline void ObjectMap::construct(Context& cx, ObjectMap* self, MetaMap* meta,
                                 Map::Kind kind) {
  Map::construct(cx, &self->baseMap(), meta, kind);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_OBJECTMAP_HPP_
