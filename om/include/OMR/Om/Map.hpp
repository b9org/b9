#ifndef OMR_OM_MAP_HPP_
#define OMR_OM_MAP_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Context.hpp>

namespace OMR {
namespace Om {

class MetaMap;

/// A description of a cell's layout, or shape. The Map is akin to a java class,
/// except that Maps are typically very small. Every Cell has a Map. Maps may be
/// shared by Cells. The MapKind can be examined to tell what kind of thing the
/// cell is.
class Map : public Cell {
 public:
  enum class Kind { SLOT_MAP, META_MAP, EMPTY_OBJECT_MAP, ARRAY_BUFFER_MAP };

  static void construct(Context& cx, Map* self, MetaMap* meta, Kind kind);

  static MetaMap* metaMap(Context& cx, const Map* self);

  Kind kind;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAP_HPP_
