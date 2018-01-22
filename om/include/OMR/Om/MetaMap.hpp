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

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  Map& baseMap() { return base().map; }

  const Map& baseMap() const { return base().map; }

  MetaMap* map() const { return baseMap().map(); }

  Base base_;
};

static_assert(std::is_standard_layout<MetaMap>::value,
              "MetaMap must be a StandardLayoutType");

#if 0
 public:
  static void construct(Context& cx, MetaMap* self) {
    // the MetaMap is it's own map.
    Map::construct(cx, self, self, Map::Kind::META_MAP);
  }
#endif

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_METAMAP_HPP_
