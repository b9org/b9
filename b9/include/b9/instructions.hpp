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

  // Object Bytecodes

  NEW_OBJECT = 0x20,

  PUSH_FROM_OBJECT = 0x21,

  POP_INTO_OBJECT = 0x22,

  CALL_INDIRECT = 0x23,

  SYSTEM_COLLECT = 0x24,
};

inline const char *toString(ByteCode bc) {
  switch (bc) {
    case ByteCode::END_SECTION:
      return "end_section";
    case ByteCode::FUNCTION_CALL:
      return "function_call";
    case ByteCode::FUNCTION_RETURN:
      return "function_return";
    case ByteCode::PRIMITIVE_CALL:
      return "primitive_call";
    case ByteCode::JMP:
      return "jmp";
    case ByteCode::DUPLICATE:
      return "duplicate";
    case ByteCode::DROP:
      return "drop";
    case ByteCode::PUSH_FROM_VAR:
      return "push_from_var";
    case ByteCode::POP_INTO_VAR:
      return "pop_into_var";
    case ByteCode::INT_ADD:
      return "int_add";
    case ByteCode::INT_SUB:
      return "int_sub";

      // CASCON2017 - Add INT_MUL and INT_DIV here

    case ByteCode::INT_PUSH_CONSTANT:
      return "int_push_constant";
    case ByteCode::INT_NOT:
      return "int_not";
    case ByteCode::INT_JMP_EQ:
      return "int_jmp_eq";
    case ByteCode::INT_JMP_NEQ:
      return "int_jmp_neq";
    case ByteCode::INT_JMP_GT:
      return "int_jmp_gt";
    case ByteCode::INT_JMP_GE:
      return "int_jmp_ge";
    case ByteCode::INT_JMP_LT:
      return "int_jmp_lt";
    case ByteCode::INT_JMP_LE:
      return "int_jmp_le";
    case ByteCode::STR_PUSH_CONSTANT:
      return "str_push_constant";
    case ByteCode::STR_JMP_EQ:
      return "str_jmp_eq";
    case ByteCode::STR_JMP_NEQ:
      return "str_jmp_neq";
    case ByteCode::NEW_OBJECT:
      return "new_object";
    case ByteCode::PUSH_FROM_OBJECT:
      return "push_from_object";
    case ByteCode::POP_INTO_OBJECT:
      return "pop_into_object";
    case ByteCode::CALL_INDIRECT:
      return "call_indirect";
    case ByteCode::SYSTEM_COLLECT:
      return "system_collect";
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
  out << "(" << i.byteCode();

  switch (i.byteCode()) {
    // 0 parameters
    case ByteCode::END_SECTION:
    case ByteCode::DUPLICATE:
    case ByteCode::FUNCTION_RETURN:
    case ByteCode::DROP:
    case ByteCode::INT_ADD:
    case ByteCode::INT_SUB:
    case ByteCode::INT_NOT:
    case ByteCode::NEW_OBJECT:
    case ByteCode::CALL_INDIRECT:
    case ByteCode::SYSTEM_COLLECT:
      break;
    // 1 parameter
    case ByteCode::FUNCTION_CALL:
    case ByteCode::PRIMITIVE_CALL:
    case ByteCode::JMP:
    case ByteCode::PUSH_FROM_VAR:
    case ByteCode::POP_INTO_VAR:
    case ByteCode::INT_PUSH_CONSTANT:
    case ByteCode::INT_JMP_EQ:
    case ByteCode::INT_JMP_NEQ:
    case ByteCode::INT_JMP_GT:
    case ByteCode::INT_JMP_GE:
    case ByteCode::INT_JMP_LT:
    case ByteCode::INT_JMP_LE:
    case ByteCode::STR_PUSH_CONSTANT:
    case ByteCode::STR_JMP_EQ:
    case ByteCode::STR_JMP_NEQ:
    case ByteCode::PUSH_FROM_OBJECT:
    case ByteCode::POP_INTO_OBJECT:
    default:
      out << " " << i.parameter();
      break;
  }
  return out << ")";
}

}  // namespace b9

#endif  // B9_INSTRUCTIONS_HPP_
