#ifndef OMR_OM_MAP_INL_HPP_
#define OMR_OM_MAP_INL_HPP_

#include <OMR/Om/Map.hpp>

#include <OMR/Om/MetaMap.hpp>

namespace OMR {
namespace Om {

inline Map::Map(MetaMap* meta, Kind kind)
    : base_{&meta->baseMap()}, kind_(kind) {}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAP_INL_HPP_
