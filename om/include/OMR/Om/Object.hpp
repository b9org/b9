#ifndef OMR_OM_OBJECT_HPP_
#define OMR_OM_OBJECT_HPP_

#include <OMR/Om/Cell.hpp>
#include <OMR/Om/Handle.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/ObjectMap.hpp>
#include <OMR/Om/Value.hpp>

#include <type_traits>

namespace OMR {
namespace Om {

class Context;

/// An adapter class that provides macro operations on the map hierachy of an
/// object. Most notably, the ObjectMapHierarchy is iterable.
class ObjectMapHierachy {
 public:
  class Iterator {
   protected:
    friend class ObjectMapHierachy;

    explicit constexpr Iterator(ObjectMap* start) noexcept : current_(start) {}

    Iterator(const Iterator&) = default;

    constexpr ObjectMap& operator*() const noexcept { return *current_; }

    Iterator& operator++(int) noexcept {
      if (current_ == nullptr) {
        return *this;
      }

      current_ = current_->parent();
      return *this;
    }

    constexpr bool operator!=(const Iterator& rhs) const noexcept {
      return current_ == rhs.current_;
    }

   private:
    ObjectMap* current_;
  };

  constexpr ObjectMapHierachy(ObjectMap* start) : start_(start) {}

  Iterator begin() const noexcept { return Iterator(start_); }

  Iterator end() const noexcept { return Iterator(nullptr); }

 private:
  ObjectMap* start_;
};

/// A Cell with dynamically allocated slots.
struct Object {
  union Base {
    Cell cell;
  };

  static constexpr std::size_t MAX_SLOTS = 32;

  /// Allocate object with this map;
  static inline Object* allocate(Context& cx, Handle<ObjectMap> map);

  /// Allocate an empty object with a new EmptyObjectMap.
  static inline Object* allocate(Context& cx);

  static Object* clone(Context& cx, Handle<Object> base);

  // Take a layout/map transition that hasn't been cached. Before taking a new
  // transition, users should use `lookupTransition` to ensure the transition
  // hasn't been taken before. See also `transition`, a higher level call for
  // transitioning across object layouts.
  static ObjectMap* takeNewTransition(Context& cx, Handle<Object> object,
                                      Infra::Span<const SlotDescriptor> desc,
                                      std::size_t hash);

  /// Transition the object's shape by adding a set of new slots.
  /// This function will reuse cached transitions.
  static ObjectMap* transition(Context& cx, Handle<Object> object,
                               Infra::Span<const SlotDescriptor> desc,
                               std::size_t hash);

  static bool get(Context& cx, const Object* self, Id id, Value& result);

  static void get(Context& cx, const Object* self, Index index, Value& result);

  static void set(Context& cx, Object* self, Index index, Value value) noexcept;

  /// slot offset lookup
  static bool index(Context& cx, const Object* self, Id id, Index& result);

  /// Set the slot that corresponds to the id. If the slot doesn't exist,
  /// allocate the slot and assign it. The result is the address of the slot.
  /// !CAN_GC!
  static bool set(Context& cx, Object* object,
                  Infra::Span<const SlotDescriptor> desc, Value value);

  ObjectMap* lookUpTransition(Context& cx,
                              Infra::Span<const SlotDescriptor> desc,
                              std::size_t hash);

  ObjectMap* takeExistingTransition(Context& cx,
                                    Infra::Span<const SlotDescriptor> desc,
                                    std::size_t hash);

  Base& base() { return base_; }

  const Base& base() const { return base_; }

  Cell& baseCell() { return base().cell; }

  const Cell& baseCell() const { return base().cell; }

  ObjectMap* map() const {
    return reinterpret_cast<ObjectMap*>(baseCell().map());
  }

  void map(ObjectMap* m) { baseCell().map(&m->baseMap()); }

  ObjectMapHierachy mapHierarchy() const { return ObjectMapHierachy(map()); }

  Value get(Index i) const {
    assert(i < fixedSlotCount_);
    return fixedSlots_[0];
  }

  void set(Index i, Value x) {
    assert(i < fixedSlotCount_);
    fixedSlots_[i] = x;
  }

  /// True if this object has no slots.
  bool empty() const {
    return (map()->slotOffset() == 0) && (map()->slotCount() == 0);
  }

  template <typename VisitorT>
  void visit(Context& cx, VisitorT& visitor) {
    baseCell().visit(cx, visitor);

    if (empty()) {
      return;
    }

    // TODO: Only visit the active slots of an object, known using
    //   map()->index().
    // TODO: Fix object's slot walking
    // TODO: Check the CoreType of the slot before marking.
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
