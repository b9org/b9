#if !defined(OMR_OM_VALUE_HPP_)
#define OMR_OM_VALUE_HPP_

#include <OMR/Infra/BitUtilities.hpp>
#include <OMR/Infra/Double.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace OMR {
namespace Om {

/// @file
/// Boxed Values

/// A RawValue is the underlying integer type that Values encode information
/// in. The RawValue has a ValueTag and 48 bits of storage encoded in it.
using RawValue = std::uint64_t;

static_assert(sizeof(RawValue) == sizeof(double),
              "A RawValue should be wide enough to store a double.");

/// In this encoding scheme, a boxed-value is any signaling NaN. The encoded
/// value is stored in the NaN's spare bits, masked by NAN_VALUE_MASK. This
/// means that it is impossible to store a legitimate sNaN into a Value. NaN's
/// must be canonicalized before being stored.
///
/// The Tags in this namespace indicate that a RawValue is a boxed immediate.
///
/// Note that for now, the tag's VALUE and MASK are the same, but this could
/// change.
namespace BoxTag {
static constexpr RawValue VALUE = Infra::Double::SPECIAL_TAG | Infra::Double::NAN_SIGNAL_TAG;
static constexpr RawValue MASK = Infra::Double::SPECIAL_TAG | Infra::Double::NAN_SIGNAL_TAG;
}  // namespace BoxTag

/// Tags that indicate the kind of value stored in a boxed immediate.
/// The Kind tag occupies the lower 7 bits of the most significant byte in
/// the mantissa. The top bit is the NAN_SIGNAL_TAG, and is always 1 for boxed
/// values.
namespace KindTag {
static constexpr std::size_t SHIFT = 32;
static constexpr RawValue MASK = RawValue(0xEF) << SHIFT;
static constexpr RawValue INTEGER = RawValue(0x01) << SHIFT;
static constexpr RawValue POINTER = RawValue(0x02) << SHIFT;
}  // namespace KindTag

/// Tags that indicate the RawValue is boxed AND a particular value.
namespace BoxKindTag {
static constexpr RawValue MASK = BoxTag::MASK | KindTag::MASK;
static constexpr RawValue INTEGER = BoxTag::VALUE | KindTag::INTEGER;
static constexpr RawValue POINTER = BoxTag::VALUE | KindTag::POINTER;
}  // namespace BoxKindTag

static constexpr RawValue VALUE_MASK = ~BoxKindTag::MASK;

/// The canonical NaN is a positive non-signaling NaN. When a NaN double is
/// stored into a Value, the NaN is canonicalized. This is to prevent us from
/// reading true NaNs that look like NaN-boxed values.
static constexpr RawValue CANONICAL_NAN =
    Infra::Double::SPECIAL_TAG | Infra::Double::NAN_EXTRA_BITS_MASK;

/// if value is a NaN, return the corresponding quiet canonical NaN. Note that
/// currently, the NaN will lose it's sign.
static constexpr bool canonicalizeNaN(RawValue value) {
  return Infra::Double::isNaN(value) ? CANONICAL_NAN : value;
}

class Cell;

union ValueData {
  RawValue asRawValue;
  double asDouble;
  void* asPointer;
  Cell* asCellPointer;
};

static_assert(sizeof(ValueData) == sizeof(RawValue),
              "RawValue is used to store doubles.");

/// A single-slot value. Pointer width.
/// https://wingolog.org/archives/2011/05/18/value-representation-in-javascript-implementations
/// https://dxr.mozilla.org/mozilla-central/source/js/public/Value.h
/// NaN boxed value.
class Value {
 public:
  struct IntegerTag {};
  static constexpr IntegerTag integer{};

  /// zero-initialize the RawData.

  constexpr Value() : data_{0} {}

  constexpr Value(const Value& other) : data_{other.raw()} {}

  explicit constexpr Value(RawValue raw) : data_{raw} {}

  constexpr Value(IntegerTag, std::int32_t integer)
      : data_{BoxKindTag::INTEGER | (integer & VALUE_MASK)} {}

  /// Get the underlying raw storage.
  constexpr RawValue raw() const noexcept { return data_.asRawValue; }

  /// Set the underlying raw storage.
  Value& raw(RawValue raw) noexcept {
    data_.asRawValue = raw;
    return *this;
  }

  bool isBoxedValue() const noexcept {
    return (raw() & BoxTag::MASK) == BoxTag::VALUE;
  }

  constexpr bool isDouble() const noexcept { return !isBoxedValue(); }

  Value& setDouble(double d) noexcept {
    if (std::isnan(d)) {
      d = Infra::Double::fromRaw(CANONICAL_NAN);
    }
    return setDoubleUnsafe(d);
  }

  Value& setDoubleUnsafe(double d) noexcept {
    data_.asDouble = d;
    return *this;
  }

  double getDouble() const noexcept {
    assert(isDouble());
    return data_.asDouble;
  }

  bool isPtr() const noexcept {
    return (raw() & BoxKindTag::MASK) == BoxKindTag::POINTER;
  }

  template <typename T = void>
  T* getPtr() const noexcept {
    assert(isPtr());
    return (T*)(raw() & VALUE_MASK);
  }

  template <typename T>
  Value& setPtr(T* ptr) noexcept {
    assert(Infra::areNoBitsSet(RawValue(ptr), ~VALUE_MASK));
    return raw(BoxKindTag::POINTER | RawValue(ptr));
  }

  bool isInteger() const noexcept {
    return Infra::isTagged(raw(), BoxKindTag::INTEGER, BoxKindTag::MASK);
  }

  std::uint32_t getInteger() const noexcept {
    assert(isInteger());
    return std::uint32_t(raw() & VALUE_MASK);
  }

  Value& setInteger(std::uint32_t value) noexcept {
    return raw(BoxKindTag::INTEGER | RawValue(value));
  }

  explicit operator RawValue() const noexcept { return data_.asRawValue; }

  Value& operator=(const Value& rhs) noexcept { return raw(rhs.raw()); }

  constexpr bool operator==(const Value& rhs) const noexcept {
    return raw() == rhs.raw();
  }

  constexpr bool operator!=(const Value& rhs) const noexcept {
    return raw() != rhs.raw();
  }

 private:
  ValueData data_;
};

static_assert(
    sizeof(RawValue) == sizeof(Value),
    "A Value is a thin struct wrapper around it's base storage type.");

inline std::ostream& operator<<(std::ostream& out, const Value& v) {
  if (v.isDouble()) {
    return out << v.getDouble();
  } else if (v.isInteger()) {
    return out << v.getInteger();
  } else if (v.isPtr()) {
    return out << v.getPtr();
  } else {
    return out << v.raw() << "!";
  }
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_VALUE_HPP_
