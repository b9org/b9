#ifndef OMR_OM_MAP_HPP_
#define OMR_OM_MAP_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Context.hpp>

namespace OMR {
namespace Om {

/// A description of a cell's layout, or shape. The Map is akin to a java class,
/// except that Maps are typically very small. Every Cell has a Map. Maps may be
/// shared by Cells. The MapKind can be examined to tell what kind of thing the
/// cell is.
struct Map {
  enum class Kind {
    SLOT_MAP, META_MAP, EMPTY_OBJECT_MAP, ARRAY_BUFFER_MAP 
  };

  union Base {
    Cell cell;
  };

  Base base;
  Kind kind;
};

static_assert(std::is_standard_layout<Map>::value, "Map must be a StandardLayoutType.");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAP_HPP_
