#ifndef B9_INSTRUCTION_HPP_
#define B9_INSTRUCTION_HPP_

#include <b9/bytecodes.hpp>
#include <cstdint>
#include <ostream>

namespace b9 {

using RawInstruction = std::uint32_t;
using Parameter = std::int32_t;  // even though only 24 bits used

/// A RawInstruction wrapper that will encode and decode instruction bytecodes
/// and immediate parameters. The Instruction layout is:
/// ```
/// |0000-0000 0000-0000 0000-0000 0000-0000
/// |---------| bytecode (8bits)
///           |-----------------------------| parameter (24bits)
/// ```
class Instruction {
 public:
  constexpr Instruction() noexcept : raw_{0} {}

  constexpr Instruction(RawInstruction raw) noexcept : raw_{raw} {}

  constexpr Instruction(ByteCode bc) noexcept
      : raw_{RawInstruction(bc) << BYTECODE_SHIFT} {}

  constexpr Instruction(ByteCode bc, Parameter p) noexcept
      : raw_{(RawInstruction(bc) << BYTECODE_SHIFT) | (p & PARAMETER_MASK)} {}

  constexpr Instruction &set(ByteCode bc, Parameter p) noexcept {
    raw_ = (RawInstruction(bc) << BYTECODE_SHIFT) | (p & PARAMETER_MASK);
    return *this;
  }

  /// Encode the bytecode
  constexpr Instruction &byteCode(ByteCode bc) noexcept {
    raw_ = (RawByteCode(bc) << BYTECODE_SHIFT) | (raw_ & PARAMETER_MASK);
    return *this;
  }

  /// Decode the bytecode
  constexpr ByteCode byteCode() const noexcept {
    return static_cast<ByteCode>(raw_ >> BYTECODE_SHIFT);
  }

  /// Encode the parameter
  constexpr Instruction &parameter(Parameter p) noexcept {
    raw_ = (raw_ & BYTECODE_MASK) | (p & PARAMETER_MASK);
    return *this;
  }

  /// Decode the parameter
  constexpr Parameter parameter() const noexcept {
    auto param = static_cast<Parameter>(raw_ & PARAMETER_MASK);
    // Sign extend when top (24th) bit is 1
    if (param & 0x0080'0000) param |= 0xFF00'0000;
    return param;
  }

  constexpr RawInstruction raw() const noexcept { return raw_; }

  constexpr bool operator==(const Instruction rhs) const {
    return raw_ == rhs.raw_;
  }

  constexpr bool operator!=(const Instruction rhs) const {
    return raw_ != rhs.raw_;
  }

 private:
  static constexpr const RawInstruction BYTECODE_SHIFT = 24;
  static constexpr const RawInstruction PARAMETER_MASK = 0x00FF'FFFF;
  static constexpr const RawInstruction BYTECODE_MASK = ~PARAMETER_MASK;

  RawInstruction raw_;
};

/// A special constant indicating the end of a sequence of instructions.
/// END_SECTION should be the last element in every functions bytecode array.
static constexpr Instruction END_SECTION{ByteCode::END_SECTION, 0};

/// Print an Instruction.
inline std::ostream &operator<<(std::ostream &out, Instruction i) {
  return out << "{" << i.byteCode() << ", " << i.parameter() << "}";
}

}  // namespace b9

#endif  // B9_INSTRUCTION_HPP_
