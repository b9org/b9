#if !defined(OMR_OM_OBJECTMAP_HPP_)
#define OMR_OM_OBJECTMAP_HPP_

// #include <OMR/Om/Context.hpp>
#include <OMR/Om/ArrayBuffer.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/SlotDescriptor.hpp>
#include <OMR/Om/TransitionSet.hpp>

namespace OMR {
namespace Om {

class Context;
struct SlotMap;

/// A map that describes the layout of an object. ObjectMaps are either the
/// EmptyObjectMap, or a SlotMap.
struct ObjectMap {
 public:
  union Base {
    Map map;
    Cell cell;
  };

  /// @{
  /// @group High level API

  static bool construct(Context& cx, Handle<ObjectMap> self);

  /// Create a slot map that derives base. Add the new slot map to the set of
  /// known transistions from base.
  static SlotMap* derive(Context& cx, Handle<ObjectMap> base,
                         const SlotDescriptor& desc, std::size_t hash);

  /// Look up a transition to a derived shape.
  SlotMap* lookUpTransition(Context& cx, const SlotDescriptor& desc,
                            std::size_t hash);

  /// @}

  /// @group Initialization
  /// @{

  ObjectMap(MetaMap* meta, Map::Kind kind);

  /// @}

  /// @{
  /// @group Base Accessors

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Map& baseMap() { return base().map; }

  const Map& baseMap() const { return base().map; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  /// @}

  Map::Kind kind() const { return baseMap().kind(); }

  /// @{
  /// @group GC Support

  template <typename VisitorT>
  inline void visit(Context& cx, VisitorT& visitor);

  Base base_;
  TransitionSet transitions_;
};

static_assert(std::is_standard_layout<ObjectMap>::value,
              "ObjectMap must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_OBJECTMAP_HPP_
