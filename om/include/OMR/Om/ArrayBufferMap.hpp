#if !defined(OMR_OM_ARRAYBUFFERMAP_HPP_)
#define OMR_OM_ARRAYBUFFERMAP_HPP_

#include <OMR/Om/Map.hpp>
#include <OMR/Om/MetaMap.hpp>

#include <type_traits>

namespace OMR {
namespace Om {

struct MetaMap;

struct ArrayBufferMap {
  union Base {
    Map map;
    Cell cell;
  };

  static ArrayBufferMap* allocate(Context& cx);

  explicit ArrayBufferMap(MetaMap* meta);

  Base& base() noexcept { return base_; }

  const Base& base() const noexcept { return base_; }

  Map& baseMap() noexcept { return base().map; }

  const Map& baseMap() const noexcept { return base().map; }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    base().map.visit(cx, visitor);
  }

 protected:
  Base base_;
};

static_assert(std::is_standard_layout<ArrayBufferMap>::value,
              "ArrayBufferMap must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ARRAYBUFFERMAP_HPP_
