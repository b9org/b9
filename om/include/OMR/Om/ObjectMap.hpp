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
#if 0
  static void construct(Context& cx, ObjectMap* self, MetaMap* meta,
                        Map::Kind kind);

  /// look up the ObjectMap that extends self with a new slot. Won't GC.
  /// If the transition is not present, lookup will return false. The result
  /// is always a SlotMap--an object map describing the new slot.
  static bool next(Context& cx, Handle<ObjectMap> self,
                   const SlotDescriptor& desc, SlotMap*& result);
#endif
  union Base {
    Map map;
    Cell cell;
  };

  Base base;
  MemTransitionSet transitions;
};

static_assert(std::is_standard_layout<ObjectMap>::value,
              "ObjectMap must be a StandardLayoutType");

#if 0
inline void ObjectMap::construct(Context& cx, ObjectMap* self, MetaMap* meta,
                                 Map::Kind kind) {
  Map::construct(cx, asMap(self), meta, kind);
}

inline bool ObjectMap::next(Context& cx, Handle<ObjectMap> self,
                            const SlotDescriptor& desc, SlotMap*& result) {
  bool found = MemTransitionSet::next(
      cx, {self, &ObjectMap::transitions}, desc, result);
  return found;
}
#endif

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_OBJECTMAP_HPP_
