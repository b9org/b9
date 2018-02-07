#if !defined(OMR_OBJECTMAP_INL_HPP_)
#define OMR_OBJECTMAP_INL_HPP_

#include <OMR/Om/Allocator.hpp>
#include <OMR/Om/Context.hpp>
#include <OMR/Om/Map.inl.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/TransitionSet.inl.hpp>

namespace OMR {
namespace Om {

inline ObjectMap::ObjectMap(
    MetaMap* meta, ObjectMap* parent,
    const Infra::Span<const SlotDescriptor>& descriptors)
    : baseMap_(meta, Map::Kind::OBJECT_MAP),
      parent_(parent),
      transitions_(),
      // TODO: Find a clearer way to construct a map without a parent.
      slotOffset_(parent ? (parent->slotOffset() + parent->slotWidth()) : 0),
      slotWidth_(0),
      slotCount_(descriptors.length()) {
  for (std::size_t i = 0; i < slotCount_; i++) {
    descriptors_[i] = descriptors[i];
    slotWidth_ += descriptors[i].type().width();
  }
}

inline Cell* ObjectMapInitializer::operator()(Context& cx,
                                              Cell* cell) noexcept {
  auto meta = cx.globals().metaMap();
  new (cell) ObjectMap(meta, parent, descriptors);
  return cell;
}

inline ObjectMap* ObjectMap::allocate(
    Context& cx, Handle<ObjectMap> parent,
    Infra::Span<const SlotDescriptor> descriptors) {
  ObjectMapInitializer init;
  init.parent = parent;
  init.descriptors = descriptors;

  std::size_t size = calculateAllocSize(descriptors.length());
  ObjectMap* result = BaseAllocator::allocate<ObjectMap>(cx, init, size);

  RootRef<ObjectMap> root(cx, result);
  bool ok = construct(cx, root);
  if (!ok) {
    return result = nullptr;
  } else {
    result = root.get();
  }

  return result;
}

inline ObjectMap* ObjectMap::allocate(Context& cx) {
  // TODO: Don't construct a bogus handle here, find a clearer way to do this.
  ObjectMap* parent = nullptr;
  return allocate(cx, Handle<ObjectMap>(parent),
                  Infra::Span<const SlotDescriptor>(nullptr, 0));
}

inline bool ObjectMap::construct(Context& cx, Handle<ObjectMap> self) {
  return TransitionSet::construct(cx, {self, &ObjectMap::transitions_});
}

inline ObjectMap* ObjectMap::lookUpTransition(
    Context& cx, const Infra::Span<const SlotDescriptor>& descriptors,
    std::size_t hash) {
  return transitions_.lookup(descriptors, hash);
}

inline ObjectMap* ObjectMap::derive(
    Context& cx, Handle<ObjectMap> base,
    const Infra::Span<const SlotDescriptor>& descriptors, std::size_t hash) {
  ObjectMap* derivation = ObjectMap::allocate(cx, base, descriptors);
  base->transitions_.tryStore(derivation, hash);
  // TODO: Write barrier? the object map
  return derivation;
}

template <typename VisitorT>
inline void ObjectMap::visit(Context& cx, VisitorT& visitor) {
  baseMap().visit(cx, visitor);
  visitor.edge(cx, (Cell*)this, (Cell*)parent());
  transitions_.visit(cx, visitor);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OBJECTMAP_INL_HPP_