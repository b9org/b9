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
    Map map;
    Cell cell;
  };

  Base base;
};

static_assert(std::is_standard_layout<EmptyObjectMap>::value,
              "EmptyObjectMap must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_EMPTYOBJECTMAP_HPP_
