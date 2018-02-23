#ifndef OMR_OM_MAP_HPP_
#define OMR_OM_MAP_HPP_

#include <OMR/Om/Cell.hpp>

namespace OMR {
namespace Om {

class Context;
struct MetaMap;

/// A description of a cell's layout, or shape. The Map is akin to a java class,
/// except that Maps are typically very small. Every Cell has a Map. Maps may be
/// shared by Cells. The MapKind can be examined to tell what kind of thing the
/// cell is.
struct Map {
  enum class Kind { META_MAP, OBJECT_MAP, ARRAY_BUFFER_MAP };

  union Base {
    Cell cell;
  };

  static void construct(Context& cx, Map* self, MetaMap* meta, Kind kind);

  Map(MetaMap* meta, Kind kind);

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  MetaMap* map() const { return reinterpret_cast<MetaMap*>(baseCell().map()); }

  Map& map(MetaMap* m) {
    baseCell().map(reinterpret_cast<Map*>(m));
    return *this;
  }

  Kind kind() const { return kind_; }

  Map& kind(Kind k) {
    kind_ = k;
    return *this;
  }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    baseCell().visit(cx, visitor);
  }

  Base base_;
  Kind kind_;
};

static_assert(std::is_standard_layout<Map>::value,
              "Map must be a StandardLayoutType.");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAP_HPP_
