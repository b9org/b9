#ifndef OMR_INFRA_DOUBLES_HPP_
#define OMR_INFRA_DOUBLES_HPP_

/// @file
/// Tools for working with doubles. Especially, for working with the underlying
/// representation of doubles.

#include <OMR/Infra/BitUtilities.hpp>

#include <cmath>
#include <cstdint>

namespace OMR {
namespace Infra {

/// raw integer constants for working with doubles.
namespace Double {

static_assert(sizeof(std::uint64_t) == sizeof(double),
              "Decoding and encoding expects 64 bit doubles.");

/// Decomposition masks.
/// @{
static constexpr std::uint64_t SIGN_MASK = 0x8000'0000'0000'0000ul;
static constexpr std::uint64_t EXPONENT_MASK = 0x7FF0'0000'0000'0000ul;
static constexpr std::uint64_t MANTISSA_MASK = 0x000F'FFFF'FFFF'FFFFul;
/// @}

/// NaN and +/- Inf
/// @{
/// A special value is any double where the exponent is maximized, aka all 1's.
/// Note that a special value with a mantissa of zero designates +/- infinity. A
/// Non-zero mantissa indicates NaN.
static constexpr std::uint64_t SPECIAL_TAG = EXPONENT_MASK;

/// There are two kinds of NaN's: signalling and quiet. Using a signalling NaN
/// in any floating point arithmetic will result in a FPU interrupt. Quiet NaNs
/// propagate through expressions without signaling. Quiet NaNs are
/// designated by a positive most-significant-bit in the mantissa.
static constexpr std::uint64_t NAN_QUIET_TAG = 0x0008'0000'0000'0000ul;

/// A mask for the remaining bits in a NaN after the NAN_TAG and the
/// NAN_QUIET_TAG;
static constexpr std::uint64_t NAN_EXTRA_BITS_MASK = 0x0007'FFFF'FFFF'FFFFul;
/// @}

/// True if value is any NaN. False if Inf or a valid number. A double is a NaN
/// if it's tagged special and has a non-zero mantissa.
static constexpr bool isNaN(std::uint64_t value) {
  return areAllBitsSet(value, Double::SPECIAL_TAG) &&
         areAnyBitsSet(value, Double::MANTISSA_MASK);
}

/// True for any quiet NaN.
static constexpr bool isQNaN(std::uint64_t value) {
  return isNaN(value) && areAllBitsSet(value, Double::NAN_QUIET_TAG);
}

/// True for any signaling NaN.
static constexpr bool isSNaN(std::uint64_t value) {
  return isNaN(value) && areNoBitsSet(value, Double::NAN_QUIET_TAG);
}

/// Reinterpret std::uint64_t to a double
static constexpr double fromRaw(std::uint64_t value) {
  union {
    double asDouble;
    std::uint64_t asRaw;
  } result{0};
  result.asRaw = value;
  return result.asDouble;
}

/// Reinterpret a double as a std::uint64_t.
static constexpr std::uint64_t toRaw(double d) {
  union {
    double asDouble;
    std::uint64_t asRaw;
  } result{0};
  result.asDouble = d;
  return result.asRaw;
}

}  // namespace Double
}  // namespace Infra
}  // namespace OMR

#endif  // OMR_INFRA_DOUBLEUTILITIES_HPP_
