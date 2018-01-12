#ifndef OMR_OM_MAP_INL_HPP_
#define OMR_OM_MAP_INL_HPP_

#include <OMR/Om/Map.hpp>

namespace OMR {
namespace Om {

inline void Map::construct(Context& cx, Map* self, Kind kind) {
  MetaMap* meta = cx->globals().metaMap;
  Cell::construct(cx, self, meta);
  self->kind = kind;
}

inline MetaMap* Map::metaMap(Context& cx, const Map* self) const {
  return static_cast<MetaMap*>(self->map);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAP_INL_HPP_
