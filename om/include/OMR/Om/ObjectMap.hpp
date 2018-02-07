#if !defined(OMR_OM_OBJECTMAP_HPP_)
#define OMR_OM_OBJECTMAP_HPP_

#include <OMR/Infra/Span.hpp>
#include <OMR/Om/ArrayBuffer.hpp>
#include <OMR/Om/Initializer.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/SlotDescriptor.hpp>
#include <OMR/Om/TransitionSet.hpp>

namespace OMR {
namespace Om {

class Context;

using Index = std::size_t;

/// The result of a slot search or iteration operation. Slots are of varying
/// width and types. The SlotLookup tells you everything you need to know to
/// work with a slot.
struct SlotLookup {
  SlotDescriptor* descriptor;
  std::size_t offset;
};

/// Like `SlotLookup`, but the located SlotDescriptors are immutable.
struct ConstSlotLookup {
  const SlotDescriptor* descriptor;
  std::size_t offset;
};

/// An iterator that also tracks slot offsets using the width of the
/// slot's type.
class SlotDescriptorIterator {
 public:
  SlotDescriptorIterator(SlotDescriptor* current, std::size_t offset = 0)
      : current_(current), offset_(offset) {}

  SlotDescriptorIterator operator++(int) {
    SlotDescriptorIterator copy(*this);
    offset_ += current_->type().width();
    ++current_;
    return copy;
  }

  SlotDescriptorIterator& operator++() {
    offset_ += current_->type().width();
    current_++;
    return *this;
  }

  bool operator==(const SlotDescriptorIterator& rhs) const {
    return (current_ == rhs.current_);
  }

  bool operator!=(const SlotDescriptorIterator& rhs) const {
    return !(*this == rhs);
  }

  SlotLookup operator*() const { return {current_, offset_}; }

 private:
  SlotDescriptor* current_;
  std::size_t offset_;
};

/// Like `SlotDescriptorIterator`, but the slot descriptors are immutable.
class ConstSlotDescriptorIterator {
 public:
  ConstSlotDescriptorIterator(const SlotDescriptor* current,
                              std::size_t offset = 0)
      : current_(current), offset_(offset) {}

  ConstSlotDescriptorIterator operator++(int) {
    offset_ += current_->type().width();
    current_++;
    return *this;
  }

  ConstSlotDescriptorIterator& operator++() {
    offset_ += current_->type().width();
    current_++;
    return *this;
  }

  bool operator==(const ConstSlotDescriptorIterator& rhs) const {
    return (current_ == rhs.current_);
  }

  bool operator!=(const ConstSlotDescriptorIterator& rhs) const {
    return !(*this == rhs);
  }

  ConstSlotLookup operator*() const { return {current_, offset_}; }

 private:
  const SlotDescriptor* current_;
  std::size_t offset_;
};

/// An iterable view of the SlotDescriptors stored in an ObjectMap.
struct ObjectMapSlotDescriptors {
  ObjectMapSlotDescriptors(Infra::Span<SlotDescriptor> descriptors,
                           std::size_t offset, std::size_t width)
      : descriptors_(descriptors), offset_(offset), width_(width) {}

  SlotDescriptorIterator begin() const {
    return SlotDescriptorIterator(descriptors_.begin(), offset_);
  }

  SlotDescriptorIterator end() const {
    return SlotDescriptorIterator(descriptors_.end(), offset_ + width_);
  }

  Infra::Span<SlotDescriptor> span() const {
    return descriptors_;
  }

 private:
  const Infra::Span<SlotDescriptor> descriptors_;
  std::size_t offset_;
  std::size_t width_;
};

/// An iterable view of the SlotDescriptors stored in an ObjectMap. The
/// ObjectMap (and it's `SlotDescriptor`s) are immutable.
struct ConstObjectMapSlotDescriptors {
  ConstObjectMapSlotDescriptors(Infra::Span<const SlotDescriptor> descriptors,
                                std::size_t offset, std::size_t width)
      : descriptors_(descriptors), offset_(offset), width_(width) {}

  // ConstObjectMapSlotDescriptors()
  ConstSlotDescriptorIterator begin() const {
    return ConstSlotDescriptorIterator(descriptors_.begin(), offset_);
  }

  ConstSlotDescriptorIterator end() const {
    return ConstSlotDescriptorIterator(descriptors_.end(), offset_ + width_);
  }

  Infra::Span<const SlotDescriptor> span() const {
    return descriptors_;
  }

 private:
  const Infra::Span<const SlotDescriptor> descriptors_;
  std::size_t offset_;  //< initial offset
  std::size_t width_;   //< total width
};

/// A map that describes the layout of an object.
struct ObjectMap {
 public:
  /// @{
  /// @group High level API

  /// Allocate an object map that describes one slot.
  static ObjectMap* allocate(Context& cx, Handle<ObjectMap> parent,
                             Infra::Span<const SlotDescriptor> descriptors);

  /// Allocate an object map that describes no slots
  static ObjectMap* allocate(Context& cx);

  /// Part two of initialization. @see-also ObjectMapInitializer
  static bool construct(Context& cx, Handle<ObjectMap> self);

  /// Create a slot map that derives base. Add the new slot map to the set of
  /// known transistions from base.
  static ObjectMap* derive(Context& cx, Handle<ObjectMap> base,
                           const Infra::Span<const SlotDescriptor>& desc,
                           std::size_t hash);

  /// Look up a transition to a derived shape.
  ObjectMap* lookUpTransition(Context& cx,
                              const Infra::Span<const SlotDescriptor>& desc,
                              std::size_t hash);

  /// @}

  /// @{
  /// @group Base Accessors

  Map& baseMap() noexcept { return baseMap_; }

  const Map& baseMap() const noexcept { return baseMap_; }

  Cell& baseCell() noexcept { return baseMap().baseCell(); }

  const Cell& baseCell() const noexcept { return baseMap().baseCell(); }

  /// @}

  /// @group Slot Queries
  /// @{

  /// Assign the slot descriptors. `descriptors` is an array of length
  /// `this->slotCount()`.
  ObjectMap& slotDescriptors(SlotDescriptor* descriptors) noexcept {
    this->slotWidth_ = 0;
    for (std::size_t i = 0; i < slotCount_; i++) {
      this->descriptors_[i] = descriptors[i];
      this->slotWidth_ += descriptors[i].type().width();
    }
    return *this;
  }

  ObjectMapSlotDescriptors slotDescriptors() noexcept {
    return ObjectMapSlotDescriptors({descriptors_, slotCount_}, slotOffset_,
                                    slotWidth_);
  }

  ConstObjectMapSlotDescriptors constSlotDescriptors() const noexcept {
    return ConstObjectMapSlotDescriptors({descriptors_, slotCount_},
                                         slotOffset_, slotWidth_);
  }

  ConstObjectMapSlotDescriptors slotDescriptors() const noexcept {
    return ConstObjectMapSlotDescriptors({descriptors_, slotCount_},
                                         slotOffset_, slotWidth_);
  }

  /// Number of slots described by this map.
  std::size_t slotCount() const noexcept { return slotCount_; }

  /// The offset into the object, where the map's slots actually begin.
  /// This is the total size of the slots represented by the parent hierachy of
  /// this map.
  std::size_t slotOffset() const noexcept { return slotOffset_; }

  /// Total footprint of slots in an object.
  std::size_t slotWidth() const noexcept { return slotWidth_; }

  /// ObjectMaps form a heirachy of "inherited layouts."
  /// The parent object map describes the slots immediately preceding
  /// If this `ObjectMap` has no parent, this function returns `nullptr`.
  /// Having no parent implies the `slotOffset` is zero.
  inline ObjectMap* parent() const noexcept { return parent_; }

  /// @group GC Support
  /// @{

  template <typename VisitorT>
  inline void visit(Context& cx, VisitorT& visitor);

  constexpr std::size_t allocSize() const {
    return calculateAllocSize(slotCount());
  }

  /// @}

 protected:
  friend struct ObjectMapInitializer;
  friend struct ObjectMapOffsets;

  /// Calculate the allocation size
  static constexpr std::size_t calculateAllocSize(std::size_t slotCount) {
    return sizeof(ObjectMap) + sizeof(SlotDescriptor) * slotCount;
  }

  /// Initialize an object map that describes a slot.
  ObjectMap(MetaMap* meta, ObjectMap* parent,
            const Infra::Span<const SlotDescriptor>& slots);

  Map baseMap_;
  ObjectMap* parent_;
  TransitionSet transitions_;

  /// @group Instance Properties
  /// @{
  std::size_t slotOffset_;
  std::size_t slotWidth_;
  std::size_t slotCount_;
  /// @}

  /// Off the end of the SlotMap is storage for it's slot descriptors.
  /// We overallocate the slot map structure to provide storage for the array.
  /// `calculateAllocSize` returns
  SlotDescriptor descriptors_[0];
};

static_assert(std::is_standard_layout<ObjectMap>::value,
              "ObjectMap must be a StandardLayoutType");

/// An offset table for the jit compilers.
struct ObjectMapOffsets {
  static constexpr std::size_t baseMap = offsetof(ObjectMap, baseMap_);
  static constexpr std::size_t parent = offsetof(ObjectMap, parent_);
  /// TODO: Flush out the offsets table
};

/// A functor that performs basic initialization of an ObjectMap.
struct ObjectMapInitializer : public Initializer {
  virtual Cell* operator()(Context& cx, Cell* cell) noexcept override;

  Handle<ObjectMap> parent;
  Infra::Span<const SlotDescriptor> descriptors;
};

/// @group Conversion Functions
/// Conversions between an ObjectMap and it's base structures.
/// @{

inline Map* asMap(ObjectMap* objectMap) { return &objectMap->baseMap(); }

inline const Map* asMap(const ObjectMap* objectMap) {
  return &objectMap->baseMap();
}

inline Cell* asCell(ObjectMap* objectMap) { return &objectMap->baseCell(); }

inline const Cell* asCell(const ObjectMap* objectMap) {
  return &objectMap->baseCell();
}

inline ObjectMap* asObjectMap(Cell* cell) {
  return reinterpret_cast<ObjectMap*>(cell);
}

inline const ObjectMap* asObjectMap(const Cell* cell) {
  return reinterpret_cast<const ObjectMap*>(cell);
}

inline ObjectMap* asObjectMap(Map* map) {
  return reinterpret_cast<ObjectMap*>(map);
}

inline const ObjectMap* asObjectMap(const Map* map) {
  return reinterpret_cast<const ObjectMap*>(map);
}

/// @}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_OBJECTMAP_HPP_
