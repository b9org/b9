#if !defined(OMR_OM_ARRAYBUFFERMAP_HPP_)
#define OMR_OM_ARRAYBUFFERMAP_HPP_

#include <OMR/Om/MetaMap.hpp>

#include <type_traits>

namespace OMR {
namespace Om {

struct ArrayBufferMap {
  union Base {
    Map map;
    Cell cell;
  };

  Base base;
};

static_assert(std::is_standard_layout<ArrayBufferMap>::value,
              "ArrayBufferMap must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ARRAYBUFFERMAP_HPP_
