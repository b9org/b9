#if !defined(OMR_OM_EMPTYOBJECTMAP_HPP_)
#define OMR_OM_EMPTYOBJECTMAP_HPP_

#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/MetaMap.hpp>

namespace OMR {
namespace Om {

/// A special ObjectMap that indicates an object has no slots. Every empty
/// object has an empty object map. The EmptyObjectMap is the root of all
/// ObjectMap lineages. The EmptyObjectMap is a heap-wide-singleton.
class EmptyObjectMap : public ObjectMap {
 public:
  static void construct(Context& cx, EmptyObjectMap* self, MetaMap* meta);
};

inline void EmptyObjectMap::construct(Context& cx, EmptyObjectMap* self, MetaMap* meta) {
  ObjectMap::construct(cx, self, meta, Map::Kind::EMPTY_OBJECT_MAP);
}

}  // namespace Om
}  // namespace OMR

#endif // OMR_OM_EMPTYOBJECTMAP_HPP_
