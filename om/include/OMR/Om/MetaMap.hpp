#if !defined(OMR_OM_METAMAP_HPP_)
#define OMR_OM_METAMAP_HPP_

#include <OMR/Om/Map.hpp>

namespace OMR {
namespace Om {

/// A map that describes the shape of other Maps. The MetaMap is self
/// descriptive, IE metaMap->map() == metaMap. The MetaMap is a heap-wide
/// singleton.
class MetaMap : public Map {
 public:
  static void construct(Context& cx, MetaMap* self) {
    // the MetaMap is it's own map.
    Map::construct(cx, self, self, Map::Kind::META_MAP);
  }
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_METAMAP_HPP_
