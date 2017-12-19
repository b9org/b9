#ifndef OMR_OM_CELL_HPP_
#define OMR_OM_CELL_HPP_

#include <OMR/Om/Id.hpp>

#include <cstdint>

namespace OMR {
namespace Om {

class Map;
class Context;
class MemoryManager;

using RawCellHeader = std::uintptr_t;

class CellHeader {
 public:
  CellHeader(Map* m = nullptr) : value_(RawCellHeader(m) << MAP_SHIFT) {}

  void map(Map* m) {
    value_ = (RawCellHeader(m) << MAP_SHIFT) | (value_ & FLAGS_MASK);
  }

  Map* map() const { return (Map*)(value_ >> MAP_SHIFT); }

  void set(Map* m, std::uint8_t flags) {
    value_ = (RawCellHeader(m) << MAP_SHIFT) | (value_ & FLAGS_MASK);
  }

 private:
  static constexpr RawCellHeader FLAGS_MASK = 0xFF;
  static constexpr std::size_t MAP_SHIFT = 8;

  RawCellHeader value_;
};

/// A managed blob of memory. All Cells have a one slot header.
class Cell {
 public:
  Cell(Map* map) : header_(map) {}

  Map* map() const { return header_.map(); }

  Cell& map(Map* m) {
    header_.map(m);
    return *this;
  }

 private:
  CellHeader header_;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_CELL_HPP_
