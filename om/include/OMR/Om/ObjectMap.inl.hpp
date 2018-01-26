#if !defined(OMR_OM_OBJECTMAP_INL_HPP_)
#define OMR_OM_OBJECTMAP_INL_HPP_

#include <OMR/Om/ObjectMap.hpp>

namespace OMR {
namespace Om {

inline void ObjectMap::construct(Context& cx, ObjectMap* self, MetaMap* meta,
                                 Map::Kind kind) {
  Map::construct(cx, &self->baseMap(), meta, kind);
}

inline SlotMap* ObjectMap::lookUpTransition(Context& cx,
                                            const SlotDescriptor& desc,
                                            std::size_t hash) {
  return nullptr;
  // TODO: Implement transition lookup
}

inline SlotMap* ObjectMap::derive(Context& cx, Handle<ObjectMap> base,
                                  const SlotDescriptor& desc,
                                  std::size_t hash) {
  SlotMap* derivation = SlotMap::allocateDerivation(cx, base, desc);
  // TODO: store derivation in the transition set
  // TODO: Write barrier? the object map
  return derivation;
}

}  // namespace Om
}  // namespace OMR

#endif // OMR_OM_OBJECTMAP_INL_HPP_
