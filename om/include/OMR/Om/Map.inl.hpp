#ifndef OMR_OM_MAP_INL_HPP_
#define OMR_OM_MAP_INL_HPP_

#include <OMR/Om/Map.hpp>

namespace OMR {
namespace Om {

inline Map::Map(MetaMap* map, MapKind kind) noexcept : Cell(map), kind_(kind) {}

inline MetaMap* Map::metaMap() const { return (MetaMap*)map(); }

}  // namespace Om
} // namespace OMR

#endif // OMR_OM_MAP_INL_HPP_
