#if !defined(OMR_OM_TRANSITIONTABLE_HPP_)
#define OMR_OM_TRANSITIONTABLE_HPP_

#include <OMR/Om/ArrayBuffer.hpp>
#include <OMR/Om/Id.hpp>
#include <OMR/Om/MemHandle.hpp>
#include <OMR/Om/MemVector.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/SlotDescriptor.hpp>

#include <cstddef>

namespace OMR {
namespace Om {

class SlotMap;

/// The TransitionSet is a collection of Maps for the purpose of tracking known
/// object transitions. As objects grow slots, a chain of maps is built up. The
/// transition table tells us the existing derivations of a given map. When an
/// object grows a slot, we look to see if this layout transition has been done
/// before, so we can reuse the map. The Transition set is embedded in other
/// native objects.
class MemTransitionSet {
 public:
  struct Entry {
    SlotDescriptor desc;
    SlotMap* map;
  };

  static bool lookup(Context& cx, const MemTransitionSet* self,
                     std::size_t hash, const SlotDescriptor& desc, SlotMap*& result) {
    const auto size = MemVector<Entry>::size(cx, &self->buffer);
    for (std::size_t i = 0; i < size; i++) {
      const auto& entry = MemVector<Entry>::get(cx, &self->buffer, i);
      if (entry.desc == desc) {
        result = entry.map;
        return true;
      }
    }
    return false;
  }

  /// TODO: this should be implemented in terms of a push_back function
  /// implemented in the memvector.
  static void grow(Context& cx, const MemHandle<MemTransitionSet>& self) {
    std::size_t size = MemVector<Entry>::size(cx, &self->buffer);
        MemVector<Entry>::resize(cx, {self, &MemTransitionSet::buffer}, size);
  }

  /// Get the next shape. Will allocate a new shape if one isn't found.
  /// TODO: Actually allocate a new shape.
  static bool next(Context& cx, MemHandle<MemTransitionSet> self,
                   const SlotDescriptor& desc, SlotMap*& result) {
    result = nullptr;
    std::size_t hash = desc.hash();
    bool found = lookup(cx, self, hash, desc, result);
    return found;
  }

  // TODO: visit the shape.
  template <typename Visitor>
  static void visit(Context& cx, Visitor& visitor, MemTransitionSet& self) {}

  static void construct(Context& cx, MemHandle<MemTransitionSet> self) {
    MemVector<Entry>::construct(cx, {self, &MemTransitionSet::buffer}, 1);
    self->size = 0;
  }

  std::size_t size;
  MemVector<Entry> buffer;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_TRANSITIONTABLE_HPP_
