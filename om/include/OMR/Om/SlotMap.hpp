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
class SlotMap : public ObjectMap {
 public:
  static void construct(Context& cx, SlotMap* self, SlotMap* parent,
                        const SlotDescriptor& desc) {
    ObjectMap::construct(cx, self, Map::metaMap(cx, parent),
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

  ObjectMap* parent;
  SlotDescriptor desc;
  Index index;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_SLOTMAP_HPP_
