#if !defined(OMR_OM_MAP_TRANSITIONTABLE_HPP_)
#define OMR_OM_JUMP_TABLE_HPP_

#include <OMR/Om/Id.hpp>

#include <cstddef>

namespace OMR {
namespace Om {

class Map;

/// @file
/// As objects grow slots, a chain of maps is built up. The transition table
/// tells us the existing derivations of a given map. When an object grows a
/// slot, we look to see if this laytout transition has been done before, so we
/// can reuse the map.

struct TransitionTableEntry {
  Id id;
  Map* map;
};

/// A small, fixed size transition table.
struct TransitionTable {
  static constexpr std::size_t MAX_TRANSITIONS = 32;
  TransitionTableEntry transitions[MAX_TRANSITIONS];
};

/// The TransitionPtr is a pointer to either a single "next" map, or a table of
/// "next"s. Many maps have exactly one transition, and this gives us a compact
/// way to express that. The pointer is low-tagged with the type of reference.
class TransitionPtr {
 public:
  /// Create an empty/nil transitionPtr, which indicates there are no existing transitions.
  constexpr TransitionPtr() noexcept : data_(0) {}

  TransitionPtr(Map* map) noexcept { setMap(map); }

  TransitionPtr(TransitionTable* table) noexcept { setTable(table); }

  bool isNil() const noexcept { return (data_ & Tag::MASK) == Tag::NIL; }

  bool isMap() const noexcept { return (data_ & Tag::MASK) == Tag::MAP; }

  Map* getMap() const noexcept {
    assert(isMap());
    return reinterpret_cast<Map*>(data_ & ~Tag::MASK);
  }

  TransitionPtr& setMap(Map* map) noexcept {
    assert(map & Tag::MASK == 0);
    data_ = reinterpret_cast<std::uintptr_t>(map) | Tag::MAP;
    return *this;
  }

  bool isTable() const noexcept { return (data_ & Tag::MASK) == Tag::TABLE; }

  TransitionTable* getTable() const noexcept {
    assert(isTable());
    return reinterpret_cast<TransitionTable*>(data_ & ~Tag::MASK);
  }

  TransitionPtr& setTable(TransitionTable* table) {
    assert(table & Tag::MASK == 0);
    return *this;
  }

 private:
  /// The different constants used in low-tagging the pointer.
  struct Tag {
    static std::uintptr_t NIL = 0x00;
    static std::uintptr_t MAP = 0x01;
    static std::uintptr_t TABLE = 0x02;
    static std::uintptr_t MASK = 0x03;
  };

  std::uintptr_t data_;
};

}  // namespace Om
}  // namespace OMR

#endif // OMR_OM_TRANSITIONTABLE_HPP_
