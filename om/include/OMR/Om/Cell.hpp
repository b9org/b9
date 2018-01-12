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

/// The Cell Header. Always contained in a Cell object.
class MemCellHeader {
 public:
  static constexpr RawCellHeader FLAGS_MASK = 0xFF;
  static constexpr std::size_t MAP_SHIFT = 8;

  /// Get the map reference.
  static Map* map(MemCellHeader* self) { return (Map*)(self->value >> MAP_SHIFT); }

  /// Set the map reference.
  static void map(Context& cx, Cell* cell, MemCellHeader* self, Map* map) {
    /// TODO: write barrier on the cell.
    self->value =
        (RawCellHeader(map) << MAP_SHIFT) | (self->value & FLAGS_MASK);
  }

  static std::uint8_t flags(MemCellHeader* self) { return std::uint8_t(self->value & FLAGS_MASK); }

  /// Set the map and the flags.
  /// TODO: Write barrier the map ref / cell.
  static void set(Context& cx, Cell* cell, MemCellHeader* self, Map* m,
                  std::uint8_t flags) {
    self->value = (RawCellHeader(m) << MAP_SHIFT) | (self->value & FLAGS_MASK);
  }

  static void construct(Context& cx, Cell* cell, MemCellHeader* self,
                        Map* map = nullptr, std::uint8_t flags = 0) {
    set(cx, cell, self, map, flags);
  }

  RawCellHeader value;
};

static_assert(std::is_standard_layout<MemCellHeader>::value, "MemCellHeader must be a StandardLayoutType.");

/// A managed blob of memory. All Cells have a one slot header.
class Cell {
 public:
  static void construct(Context& cx, Cell* self, Map* map = nullptr,
                        std::uint8_t flags = 0) {
    MemCellHeader::construct(cx, self, &self->header, map, flags);
  }

  static Map* map(Cell* self) { return MemCellHeader::map(&self->header); }

  static Map* map(Context& cx, Map* map) {
    MemCellHeader::map(cx, {this, &Cell::header}, map);
  }

  MemCellHeader header;
};

static_assert(std::is_standard_layout<Cell>::value, "Cell must be a StandardLayoutType");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_CELL_HPP_
