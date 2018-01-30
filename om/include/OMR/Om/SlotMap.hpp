#if !defined(OMR_OM_SLOTMAP_HPP_)
#define OMR_OM_SLOTMAP_HPP_

#include <OMR/Om/Handle.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/MetaMap.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/SlotDescriptor.hpp>
#include <cassert>

namespace OMR {
namespace Om {

class Context;

using Index = std::uint8_t;

/// The SlotMap is an ObjectMap that describes a slot (aka field or property) of
/// an object.
struct SlotMap {
 public:
  union Base {
    ObjectMap objectMap;
    Map map;
    Cell cell;
  };

  static SlotMap* allocate(Context& cx, Handle<ObjectMap> parent,
                           const SlotDescriptor& desc);

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  Map& baseMap() { return base().map; }

  const Map& baseMap() const { return base().map; }

  ObjectMap& baseObjectMap() { return base().objectMap; }

  const ObjectMap& baseObjectMap() const { return base().objectMap; }

  Map::Kind kind() const { return baseMap().kind(); }

  Index index() const { return index_; }

  ObjectMap* parent() const { return parent_; }

  SlotMap& parent(ObjectMap* p) {
    parent_ = p;
    return *this;
  }

  const SlotDescriptor& slotDescriptor() const { return desc_; };

  SlotMap& slotDescriptor(const SlotDescriptor& d) {
    desc_ = d;
    return *this;
  }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    baseObjectMap().visit(cx, visitor);
    visitor.edge(cx, &baseCell(), &parent()->baseCell());
  }

 protected:
  Base base_;
  ObjectMap* parent_;
  SlotDescriptor desc_;
  Index index_;

 private:
  static bool construct(Context& cx, Handle<SlotMap> self);

  SlotMap(ObjectMap* parent, SlotDescriptor& desc);
};

static_assert(std::is_standard_layout<SlotMap>::value,
              "SlotMap must be a StandardLayoutType.");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_SLOTMAP_HPP_
