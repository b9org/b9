#ifndef OMR_OM_OBJECT_HPP_
#define OMR_OM_OBJECT_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/EmptyObjectMap.hpp>
#include <OMR/Om/Handle.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/SlotMap.hpp>
#include <OMR/Om/Value.hpp>

#include <type_traits>

namespace OMR {
namespace Om {

class Context;

/// A Cell with dynamically allocated slots.
struct Object {
  union Base {
    Cell cell;
  };

  /// Allocate empty object.
  static inline Object* allocate(Context& cx, Handle<EmptyObjectMap> map);

  /// Allocate object with corresponding slot map;
  static inline Object* allocate(Context& cx, Handle<SlotMap> map);

  /// Allocate an empty object with a new EmptyObjectMap.
  static inline Object* allocate(Context& cx);

  static Object* clone(Context& cx, Handle<Object> base);

  static SlotMap* takeNewTransition(Context& cx, Handle<Object> object,
                                    const SlotDescriptor& desc,
                                    std::size_t hash);

  // take an existing transition or take a new one if we have to.
  static SlotMap* transition(Context& cx, Handle<Object> object,
                             const SlotDescriptor& desc, std::size_t hash);

  static bool get(Context& cx, const Object* self, Id id, Value& result);

  static void get(Context& cx, const Object* self, Index index, Value& result);

  static void set(Context& cx, Object* self, Index index, Value value) noexcept;

  static bool index(Context& cx, const Object* self, const SlotDescriptor& desc,
                    Index& result);

  /// Set the slot that corresponds to the id. If the slot doesn't exist,
  /// allocate the slot and assign it. The result is the address of the slot.
  /// !CAN_GC!
  static bool set(Context& cx, Object* object, const SlotDescriptor& desc,
                  Value value);

#if 0
  /// Allocate a new slot corresponding to the id. The object may not already
  /// have a slot with this Id matching. !CAN_GC!
  static Index newSlot(Context& cx, Handle<Object> self, Id id);
#endif

  static constexpr Index MAX_SLOTS = 32;

  SlotMap* lookUpTransition(Context& cx, const SlotDescriptor& desc,
                            std::size_t hash);

  SlotMap* takeExistingTransition(Context& cx, const SlotDescriptor& desc,
                                  std::size_t hash);

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  ObjectMap* map() const {
    return reinterpret_cast<ObjectMap*>(baseCell().map());
  }

  void map(ObjectMap* m) { baseCell().map(&m->baseMap()); }

  Value get(Index i) const {
    assert(i < fixedSlotCount_);
    return fixedSlots_[0];
  }

  void set(Index i, Value x) {
    assert(i < fixedSlotCount_);
    fixedSlots_[i] = x;
  }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    baseCell().visit(cx, visitor);

    if (map()->kind() == Map::Kind::EMPTY_OBJECT_MAP) {
      return;
    }

    // TODO: Only visit the active slots of an object, known using
    // map()->index().
    for (Value val : fixedSlots_) {
      if (val.isPtr()) {
        visitor.edge(cx, (Cell*)this, val.getPtr<Cell>());
      }
    }
  }

 protected:
  friend struct ObjectInitializer;

  Base base_;
  // TODO: Dynamic slots in objects: MemVector<Value> dynamicSlots;
  // TODO: Variable number of fixed slots in objects
  std::size_t fixedSlotCount_;
  Value fixedSlots_[32];

 private:
  static void construct(Context& cx, Handle<Object> self,
                        Handle<ObjectMap> map);
};

static_assert(std::is_standard_layout<Object>::value,
              "Object must be a StandardLayoutType.");

}  // namespace Om
}  // namespace OMR

#endif  // B9_OBJECTS_HPP_
