#if !defined(OMR_OM_SLOTMAP_INL_HPP_)
#define OMR_OM_SLOTMAP_INL_HPP_

#include <OMR/Om/Allocator.hpp>
#include <OMR/Om/Initializer.hpp>
#include <OMR/Om/SlotMap.hpp>

namespace OMR {
namespace Om {

struct SlotMapInitializer : public Initializer {
  SlotMapInitializer(Handle<ObjectMap> parent, const SlotDescriptor& desc)
      : parent_(parent), desc_(desc) {}

  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto m = reinterpret_cast<SlotMap*>(cell);
    m->baseMap().map(cx.globals().metaMap);
    m->baseMap().kind(Map::Kind::SLOT_MAP);
    m->parent(parent_.get());
    m->slotDescriptor(desc_);
    // TODO: Set the SlotMap's index!!!
    return &m->baseCell();
  }

  Handle<ObjectMap> parent_;
  SlotDescriptor desc_;
};

inline SlotMap* SlotMap::allocate(Context& cx, Handle<ObjectMap> parent,
                                  const SlotDescriptor& desc) {
  SlotMapInitializer init(parent, desc);
  RootRef<SlotMap> root(cx, BaseAllocator::allocate<SlotMap>(cx, init));
  bool ok = construct(cx, root);
  if (!ok) {
    return nullptr;
  }

  return root.get();
}

inline bool SlotMap::construct(Context& cx, Handle<SlotMap> self) {
  return ObjectMap::construct(cx, self.reinterpret<ObjectMap>());
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_SLOTMAP_INL_HPP_
