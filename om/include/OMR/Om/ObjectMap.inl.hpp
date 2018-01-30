#if !defined(OMR_OM_OBJECTMAP_INL_HPP_)
#define OMR_OM_OBJECTMAP_INL_HPP_

#include <OMR/Om/Allocator.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/TransitionSet.inl.hpp>

namespace OMR {
namespace Om {

inline ObjectMap::ObjectMap(MetaMap* meta, Map::Kind kind)
    : base_{{meta, kind}} {}

inline bool ObjectMap::construct(Context& cx, Handle<ObjectMap> self) {
  // Map::construct(cx, self.reinterpret<Map>());
  return TransitionSet::construct(cx, {self, &ObjectMap::transitions_});
}

inline SlotMap* ObjectMap::lookUpTransition(Context& cx,
                                            const SlotDescriptor& desc,
                                            std::size_t hash) {
  return transitions_.lookup(desc, hash);
}

inline SlotMap* ObjectMap::derive(Context& cx, Handle<ObjectMap> base,
                                  const SlotDescriptor& desc,
                                  std::size_t hash) {
  SlotMap* derivation = SlotMap::allocate(cx, base, desc);
  base->transitions_.tryStore(derivation, hash);
  // TODO: Write barrier? the object map
  return derivation;
}

template <typename VisitorT>
inline void ObjectMap::visit(Context& cx, VisitorT& visitor) {
  baseMap().visit(cx, visitor);
  transitions_.visit(cx, visitor);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_OBJECTMAP_INL_HPP_
