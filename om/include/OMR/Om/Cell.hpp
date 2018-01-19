#ifndef OMR_OM_CELL_HPP_
#define OMR_OM_CELL_HPP_

#include <OMR/Om/Id.hpp>

#include <cstdint>

namespace OMR {
namespace Om {

class Cell;
class Map;

class Context;

class MemoryManager;

using RawCellHeader = std::uintptr_t;

/// The Cell Header. Always contained in a Cell object. The operations are not
/// read/write barriered.
struct CellHeader {
  static constexpr RawCellHeader FLAGS_MASK = 0xFF;
  static constexpr std::size_t MAP_SHIFT = 8;

  /// Get the map reference.
  Map* map() const { return (Map*)(value >> MAP_SHIFT); }

  /// Set the map reference. No write barrier.
  void map( Map* map) {
    value = (RawCellHeader(map) << MAP_SHIFT) | (value & FLAGS_MASK);
  }

  std::uint8_t flags() const { return std::uint8_t(value & FLAGS_MASK); }

  /// Set the map and the flags.
  void set(Map* m, std::uint8_t flags) {
    value = (RawCellHeader(m) << MAP_SHIFT) | (flags & FLAGS_MASK);
  }

  RawCellHeader value;
};

static_assert(std::is_standard_layout<CellHeader>::value,
              "CellHeader must be a StandardLayoutType.");

/// A managed blob of memory. All Cells have a one slot header.
struct Cell {

  static void construct(Context& cx, Cell* self, Map* map, std::uint8_t flags = 0);

  CellHeader header;
};

static_assert(std::is_standard_layout<Cell>::value,
              "Cell must be a StandardLayoutType");


inline void Cell::construct(Context&cx, Cell* self, Map* map, std::uint8_t flags) {
  self->header.set(map, flags);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_CELL_HPP_
