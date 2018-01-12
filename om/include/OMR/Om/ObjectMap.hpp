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
class ObjectMap : public Map {
 public:

  static void construct(Context& cx, ObjectMap* self, MetaMap* meta, Map::Kind kind);

  /// look up the ObjectMap that extends self with a new slot. Won't GC.
  /// If the transition is not present, lookup will return false. The result
  /// is always a SlotMap--an object map describing the new slot.
  static bool next(Context& cx, Handle<ObjectMap> self,
                   const SlotDescriptor& desc, SlotMap*& result);

  MemTransitionSet transitions;
};

inline bool ObjectMap::next(Context& cx, Handle<ObjectMap> self,
                  const SlotDescriptor& desc, SlotMap*& result) {
  bool found = MemTransitionSet::next(cx, {self, &ObjectMap::transitions}, desc, result);
  return found;
}

inline void ObjectMap::construct(Context& cx, ObjectMap* self, MetaMap* meta, Map::Kind kind) {
  Map::construct(cx, self, meta, kind);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_OBJECTMAP_HPP_
