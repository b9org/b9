#if !defined(OMR_OM_SLOTMAP_INL_HPP_)
#define OMR_OM_SLOTMAP_INL_HPP_

#include <OMR/Om/SlotMap.hpp>

#include <Allocator.hpp> // glue

namespace OMR {
namespace Om {

inline SlotMap* SlotMap::allocateDerivation(Context& cx,
                                            Handle<ObjectMap> parent,
                                            const SlotDescriptor& desc) {
  return allocateSlotMap(cx, parent, desc);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_SLOTMAP_INL_HPP_
