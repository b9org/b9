#if !defined(OMR_OM_ARRAYBUFFERMAP_HPP_)
#define OMR_OM_ARRAYBUFFERMAP_HPP_

#include <OMR/Om/MetaMap.hpp>

namespace OMR {
namespace Om {

class ArrayBufferMap : public Map {
 public:
  static void construct(Context& cx, ArrayBufferMap* self, MetaMap* meta);
};

inline void ArrayBufferMap::construct(Context& cx, ArrayBufferMap* self,
                                      MetaMap* meta) {
  Map::construct(cx, self, meta, Map::Kind::ARRAY_BUFFER_MAP);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ARRAYBUFFERMAP_HPP_
