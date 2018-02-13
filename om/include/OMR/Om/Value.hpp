#if !defined(OMR_OM_VALUE_HPP_)
#define OMR_OM_VALUE_HPP_

#include <OMR/Infra/BitUtilities.hpp>
#include <OMR/Infra/Double.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

namespace OMR {
namespace Om {

/// @file
/// Boxed Values

/// Values are encoded in floating point NaNs, using the set on NaNs not used
/// for normal FP math.  All NaN FP values must have an exponent of -1.  The set
/// of NaNs we use are known as signaling NaNs, which means the quiet bit must
/// always be set to 0.  We set the `kind` in the tag to distinguish different
/// kinds of Values, e.g. Pointers from Integers.  To make sure NULL Values are
/// distiniguished from inifinity floating point numbers, the kind must be
/// non-zero.
///
/// The breakdown is:
/// NaN box:   12 bits
///  sign:        01 bits
///  exponent:    11 bits
/// tag:       04 bits
///  quiet bit:   01 bits
///  value kind:  03 bits
/// Value:     48 bits

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
namespace BoxTag {
static constexpr RawValue VALUE = Infra::Double::SPECIAL_TAG;
static constexpr RawValue MASK =
    Infra::Double::SPECIAL_TAG | Infra::Double::NAN_QUIET_TAG;
}  // namespace BoxTag

/// Tags that indicate the kind of value stored in a boxed immediate.
/// The Kind tag occupies the lower 3 bits of the most significant nibble in
/// the mantissa. The top bit is the NAN_QUIET_TAG, and is always 0 for boxed
/// values. The
namespace KindTag {
static constexpr std::size_t SHIFT = 42;
static constexpr RawValue MASK = RawValue(0x7) << SHIFT;  // 3 low bits
static constexpr RawValue INTEGER = RawValue(0x0) << SHIFT;
static constexpr RawValue POINTER = RawValue(0x1) << SHIFT;
}  // namespace KindTag

static_assert((BoxTag::MASK & KindTag::MASK) == 0,
              "Two masks must not overlap");

/// Tags that indicate the RawValue is boxed AND a particular value.
namespace BoxKindTag {
static constexpr RawValue MASK = BoxTag::MASK | KindTag::MASK;
static constexpr RawValue INTEGER = BoxTag::VALUE | KindTag::INTEGER;
static constexpr RawValue POINTER = BoxTag::VALUE | KindTag::POINTER;
}  // namespace BoxKindTag

static constexpr RawValue VALUE_MASK = ~BoxKindTag::MASK;

/// The canonical NaN is a positive non-signaling NaN. When a NaN double is
/// stored into a Value, the NaN is canonicalized.  Doing so ensures that the
/// NaN is made quiet and unsigned. This is to prevent us from reading true NaNs
/// that look like NaN-boxed values.
static constexpr RawValue CANONICAL_NAN = Infra::Double::SPECIAL_TAG |
                                          Infra::Double::NAN_QUIET_TAG |
                                          Infra::Double::NAN_EXTRA_BITS_MASK;

/// if value is a NaN, return the canonical NaN.
static constexpr RawValue canonicalizeNaN(RawValue value) {
  if (Infra::Double::isNaN(value)) {
    return CANONICAL_NAN;
  }
  return value;
}

static constexpr double canonicalizeNaN(double value) {
  return Infra::Double::fromRaw(canonicalizeNaN(Infra::Double::toRaw(value)));
}

struct Cell;

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
  /// zero-initialize the RawData.
  constexpr Value() : data_{0} {}

  constexpr Value(const Value& other) : data_{other.raw()} {}

  // explicit constexpr Value(RawValue r) : data_{r} {}

  explicit constexpr Value(std::int32_t i)
      : data_{BoxKindTag::INTEGER | (RawValue(i) & VALUE_MASK)} {}

  template <typename T>
  explicit Value(T* p)
      : data_{BoxKindTag::POINTER |
              (reinterpret_cast<RawValue>(p) & VALUE_MASK)} {
    assert((reinterpret_cast<RawValue>(p) & ~VALUE_MASK) == 0);
  }

  explicit Value(double d) { setDouble(d); }

  /// Get the underlying raw storage.
  constexpr RawValue raw() const noexcept { return data_.asRawValue; }

  /// Set the underlying raw storage.
  Value& raw(RawValue raw) noexcept {
    data_.asRawValue = raw;
    return *this;
  }

  bool constexpr isBoxedValue() const noexcept {
    return Infra::Double::isSNaN(raw());
  }

  constexpr bool isDouble() const noexcept { return !isBoxedValue(); }

  Value& setDouble(double d) noexcept {
    return setDoubleUnsafe(canonicalizeNaN(d));
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
    return out << "(double " << v.getDouble() << ")";
  } else if (v.isInteger()) {
    return out <<"(integer " << v.getInteger() << ")";
  } else if (v.isPtr()) {
    return out << "(ptr " << v.getPtr() << ")";
  } else {
    return out << "(unknown" << v.raw() << ")";
  }
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_VALUE_HPP_
