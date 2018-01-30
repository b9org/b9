#if !defined(OMR_OM_METAMAP_HPP_)
#define OMR_OM_METAMAP_HPP_

#include <OMR/Om/Map.hpp>

#include <type_traits>

namespace OMR {
namespace Om {

/// A map that describes the shape of other Maps. The MetaMap is self
/// descriptive, IE metaMap->map() == metaMap. The MetaMap is a heap-wide
/// singleton.
struct MetaMap {
  union Base {
    Map map;
    Cell cell;
  };

  static MetaMap* allocate(Context& cx);

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  Map& baseMap() { return base().map; }

  const Map& baseMap() const { return base().map; }

  MetaMap* map() const { return baseMap().map(); }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    baseMap().visit(cx, visitor);
  }

  Base base_;
};

static_assert(std::is_standard_layout<MetaMap>::value,
              "MetaMap must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_METAMAP_HPP_
