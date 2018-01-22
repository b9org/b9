#if !defined(OMR_OM_EMPTYOBJECTMAP_HPP_)
#define OMR_OM_EMPTYOBJECTMAP_HPP_

#include <OMR/Om/MetaMap.hpp>
#include <OMR/Om/ObjectMap.hpp>

#include <type_traits>

namespace OMR {
namespace Om {

/// A special ObjectMap that indicates an object has no slots. Every empty
/// object has an empty object map. The EmptyObjectMap is the root of all
/// ObjectMap lineages. The EmptyObjectMap is a heap-wide-singleton.
struct EmptyObjectMap {
  union Base {
    ObjectMap objectMap;
    Map map;
    Cell cell;
  };

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  ObjectMap& baseObjectMap() { return base().objectMap; }

  const ObjectMap& baseObjectMap() const { return base().objectMap; }

  Map& baseMap() { return base().map; }

  const Map& baseMap() const { return base().map; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  Base base_;
};

static_assert(std::is_standard_layout<EmptyObjectMap>::value,
              "EmptyObjectMap must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_EMPTYOBJECTMAP_HPP_
