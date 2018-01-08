#ifndef B9_INSTRUCTIONS_HPP_
#define B9_INSTRUCTIONS_HPP_

#include <cstdint>
#include <ostream>

namespace b9 {

using RawByteCode = std::uint8_t;

enum class ByteCode : RawByteCode {

  // Generic ByteCodes

  // A special uninterpreted ByteCode placed the end of a
  // ByteCode array.
  END_SECTION = 0x0,

  // Call a Base9 function
  FUNCTION_CALL = 0x1,
  // Return from a function
  FUNCTION_RETURN = 0x2,
  // Call a native C function
  PRIMITIVE_CALL = 0x3,
  // Jump unconditionally by the offset
  JMP = 0x4,
  // Duplicate the top element on the stack
  DUPLICATE = 0x5,
  // Drop the top element of the stack
  DROP = 0x6,
  // Push from a local variable
  PUSH_FROM_VAR = 0x7,
  // Push into a local variable
  POP_INTO_VAR = 0x8,

  // Integer bytecodes

  // Add two integers
  INT_ADD = 0x9,
  // Subtract two integers
  INT_SUB = 0xa,

  // CASCON2017 - Add INT_MUL and INT_DIV here

  // Push a constant
  INT_PUSH_CONSTANT = 0xd,
  // Invert a boolean value, all non-zero integers are true
  INT_NOT = 0xe,
  // Jump if two integers are equal
  INT_JMP_EQ = 0xf,
  // Jump if two integer are not equal
  INT_JMP_NEQ = 0x10,
  // Jump if the first integer is greater than the second
  INT_JMP_GT = 0x11,
  // Jump if the first integer is greater than or equal to the second
  INT_JMP_GE = 0x12,
  // Jump if the first integer is less than to the second
  INT_JMP_LT = 0x13,
  // Jump if the first integer is less than or equal to the second
  INT_JMP_LE = 0x14,

  // String ByteCodes

  // Push a string from this module's constant pool
  STR_PUSH_CONSTANT = 0x15,
  // Jump if two strings are equal
  STR_JMP_EQ = 0x16,
  // Jump if two strings are not equal
  STR_JMP_NEQ = 0x17,
};

inline const char *toString(ByteCode bc) {
  switch (bc) {
    case ByteCode::END_SECTION:
      return "ByteCode::END_SECTION";
    case ByteCode::FUNCTION_CALL:
      return "FUNCTION_CALL";
    case ByteCode::FUNCTION_RETURN:
      return "FUNCTION_RETURN";
    case ByteCode::PRIMITIVE_CALL:
      return "ByteCode::PRIMITIVE_CALL";
    case ByteCode::JMP:
      return "JMP";
    case ByteCode::DUPLICATE:
      return "DUPLICATE";
    case ByteCode::DROP:
      return "DROP";
    case ByteCode::PUSH_FROM_VAR:
      return "ByteCode::PUSH_FROM_VAR";
    case ByteCode::POP_INTO_VAR:
      return "POP_INTO_VAR";
    case ByteCode::INT_ADD:
      return "INT_ADD";
    case ByteCode::INT_SUB:
      return "INT_SUB";

      // CASCON2017 - Add INT_MUL and INT_DIV here

    case ByteCode::INT_PUSH_CONSTANT:
      return "INT_PUSH_CONSTANT";
    case ByteCode::INT_NOT:
      return "ByteCode::INT_NOT";
    case ByteCode::INT_JMP_EQ:
      return "ByteCode::INT_JMP_EQ";
    case ByteCode::INT_JMP_NEQ:
      return "ByteCode::INT_JMP_NEQ";
    case ByteCode::INT_JMP_GT:
      return "ByteCode::INT_JMP_GT";
    case ByteCode::INT_JMP_GE:
      return "ByteCode::INT_JMP_GE";
    case ByteCode::INT_JMP_LT:
      return "ByteCode::INT_JMP_LT";
    case ByteCode::INT_JMP_LE:
      return "ByteCode::INT_JMP_LE";
    case ByteCode::STR_PUSH_CONSTANT:
      return "ByteCode::STR_PUSH_CONSTANT";
    case ByteCode::STR_JMP_EQ:
      return "ByteCode::STR_JMP_EQ";
    case ByteCode::STR_JMP_NEQ:
      return "ByteCode::STR_JMP_NEQ";
    default:
      return "UNKNOWN_BYTECODE";
  }
}

/// Print a ByteCode
inline std::ostream &operator<<(std::ostream &out, ByteCode bc) {
  return out << toString(bc);
}

using RawInstruction = std::uint32_t;

/// The 24bit immediate encoded in an instruction. Note that parameters are
/// signed values, so special care must be taken to sign extend 24bits to 32.
using Parameter = std::int32_t;

/// A RawInstruction wrapper that will encode and decode instruction bytecodes
/// and immediate parameters. The Instruction layout is:
/// ```
/// |0000-0000 0000-0000 0000-0000 0000-0000
/// |---------| bytecode (8bits)
///           |-----------------------------| parameter (24bits)
/// ```
///
/// For many ByteCodes, the parameter is unused and left as zero.
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

#endif  // B9_INSTRUCTIONS_HPP_
