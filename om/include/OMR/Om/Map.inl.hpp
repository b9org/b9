#ifndef OMR_OM_MAP_INL_HPP_
#define OMR_OM_MAP_INL_HPP_

#include <OMR/Om/Map.hpp>

namespace OMR {
namespace Om {

inline void Map::construct(Context& cx, Map* self, MetaMap* meta, Kind kind) {
  Cell::construct(cx, &self->baseCell(), &meta->baseMap());
  kind = kind;
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAP_INL_HPP_
