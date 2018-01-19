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

  static void construct(Context& cx, ObjectMap* self, MetaMap* meta,
                        Map::Kind kind);

  union Base {
    Map map;
    Cell cell;
  };

  Base base;
  MemTransitionSet transitions;
};

static_assert(std::is_standard_layout<ObjectMap>::value,
              "ObjectMap must be a StandardLayoutType");

inline void ObjectMap::construct(Context& cx, ObjectMap* self, MetaMap* meta,
                                 Map::Kind kind) {
  Map::construct(cx, &self->base.map, meta, kind);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_OBJECTMAP_HPP_
