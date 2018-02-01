#if !defined(OMR_OM_MAPDESCRIPTOR_HPP_)
#define OMR_OM_MAPDESCRIPTOR_HPP_

#include <OMR/Infra/HashUtilities.hpp>
#include <OMR/Om/Id.hpp>
#include <OMR/Om/Value.hpp>

#include <cstddef>
#include <functional>

namespace OMR {
namespace Om {

/// TODO: Move this somewhere else.
using Ref = std::uint32_t;

/// Fundamental, built in types.
/// At it's core, every SlotType is represented by one of these values.
enum class CoreType {
  /// Binary data slots, of various sizes.
  INT8,
  INT16,
  INT32,
  INT64,
  /// floating point double. 64 bits.
  DOUBLE,
  /// A polymorphic Value. It self-encodes it's type. See `Value`.
  VALUE,
  /// A reference to a managed Cell.
  REF
};

/// Calculate the size of a CoreType. Fundamental types have
constexpr std::size_t width(CoreType t) noexcept {
  switch (t) {
    case CoreType::INT8:
      return 1;
    case CoreType::INT16:
      return 2;
    case CoreType::INT32:
      return 4;
    case CoreType::INT64:
      return 8;
    case CoreType::DOUBLE:
      return 8;
    case CoreType::VALUE:
      return sizeof(Value);
    case CoreType::REF:
      return sizeof(Ref);
  }
}

class SlotType {
 public:
  constexpr SlotType(const SlotType&) = default;

  constexpr SlotType(Id id, CoreType coreType) noexcept
      : id_(id), coreType_(coreType) {}

  constexpr CoreType coreType() const noexcept { return coreType_; }

  constexpr Id id() const noexcept { return id_; }

  SlotType& id(Id id) noexcept {
    id_ = id;
    return *this;
  }

  constexpr std::size_t hash() const noexcept {
    return Infra::Hash::mix(id_.hash(), std::size_t(coreType()));
  }

  constexpr bool operator==(const SlotType& rhs) const noexcept {
    return id_ == rhs.id_;
  }

  constexpr bool operator!=(const SlotType& rhs) const noexcept {
    return id_ != rhs.id_;
  }

  constexpr std::size_t width() const noexcept {
    return ::OMR::Om::width(coreType());
  }

 private:
  Id id_;
  CoreType coreType_;
};

class SlotDescriptor {
 public:
  SlotDescriptor(const SlotDescriptor&) = default;

  SlotDescriptor(const SlotType& type, const Id& id) : type_(type), id_(id) {}

  Id id() const { return id_; }

  SlotDescriptor& id(Id id) {
    id_ = id;
    return *this;
  }

  SlotType& type() { return type_; }

  const SlotType& type() const { return type_; }

  SlotDescriptor& type(const SlotType& type) {
    type_ = type;
    return *this;
  }

  constexpr std::size_t hash() const {
    return Infra::Hash::mix(id().hash(), type().hash());
  }

  constexpr bool operator==(const SlotDescriptor& rhs) const {
    return (id_ == rhs.id_) && (type_ == rhs.type_);
  }

  constexpr bool operator!=(const SlotDescriptor& rhs) const {
    return (id_ != rhs.id_) || (type_ != rhs.type_);
  }

  /// Width of the slot's value.
  constexpr std::size_t width() const noexcept { return type_.width(); }

  /// The fundamental type of the slot.
  constexpr CoreType coreType() const noexcept { return type_.coreType(); }

 private:
  SlotType type_;
  Id id_;
};

static_assert(
    std::is_standard_layout<SlotDescriptor>::value,
    "Slot Descriptors are heap allocated, so must be StandardLayoutType.");

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAPDESCRIPTOR_HPP_
