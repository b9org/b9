#ifndef B9_INSTRUCTIONS_HPP_
#define B9_INSTRUCTIONS_HPP_

#include <cstdint>
#include <ostream>

namespace b9 {

using RawOpCode = std::uint8_t;

enum class OpCode : RawOpCode {

  // Generic ByteCodes

  // A special uninterpreted OpCode placed the end of a
  // OpCode array.
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
  PUSH_FROM_LOCAL = 0x7,
  // Pop into a local variable
  POP_INTO_LOCAL = 0x8,
  // Push from a function parameter
  PUSH_FROM_PARAM = 0x9,
  // Pop into a function parameter
  POP_INTO_PARAM = 0xa,

  // Integer opcodes

  // Add two integers
  INT_ADD = 0xb,
  // Subtract two integers
  INT_SUB = 0xc,
  // Multiply two integers
  INT_MUL = 0xd,
  // Divide two integers
  INT_DIV = 0xe,

  // Push a constant
  INT_PUSH_CONSTANT = 0xf,
  // Invert a boolean value, all non-zero integers are true
  INT_NOT = 0x10,
  // Jump if two integers are equal
  INT_JMP_EQ = 0x11,
  // Jump if two integer are not equal
  INT_JMP_NEQ = 0x12,
  // Jump if the first integer is greater than the second
  INT_JMP_GT = 0x13,
  // Jump if the first integer is greater than or equal to the second
  INT_JMP_GE = 0x14,
  // Jump if the first integer is less than to the second
  INT_JMP_LT = 0x15,
  // Jump if the first integer is less than or equal to the second
  INT_JMP_LE = 0x16,

  // String ByteCodes

  // Push a string from this module's constant pool
  STR_PUSH_CONSTANT = 0x17,
  // Jump if two strings are equal
  STR_JMP_EQ = 0x18,
  // Jump if two strings are not equal
  STR_JMP_NEQ = 0x19,
  
  // Object Bytecodes

  NEW_OBJECT = 0x20,

  PUSH_FROM_OBJECT = 0x21,

  POP_INTO_OBJECT = 0x22,

  CALL_INDIRECT = 0x23,

  SYSTEM_COLLECT = 0x24,
};

inline const char *toString(OpCode bc) {
  switch (bc) {
    case OpCode::END_SECTION:
      return "end_section";
    case OpCode::FUNCTION_CALL:
      return "function_call";
    case OpCode::FUNCTION_RETURN:
      return "function_return";
    case OpCode::PRIMITIVE_CALL:
      return "primitive_call";
    case OpCode::JMP:
      return "jmp";
    case OpCode::DUPLICATE:
      return "duplicate";
    case OpCode::DROP:
      return "drop";
    case OpCode::PUSH_FROM_LOCAL:
      return "push_from_local";
    case OpCode::POP_INTO_LOCAL:
      return "pop_into_local";
    case OpCode::PUSH_FROM_PARAM:
      return "push_from_param";
    case OpCode::POP_INTO_PARAM:
      return "pop_into_param";
    case OpCode::INT_ADD:
      return "int_add";
    case OpCode::INT_SUB:
      return "int_sub";
    case OpCode::INT_MUL:
      return "int_mul";
    case OpCode::INT_DIV:
      return "int_div";
    case OpCode::INT_PUSH_CONSTANT:
      return "int_push_constant";
    case OpCode::INT_NOT:
      return "int_not";
    case OpCode::INT_JMP_EQ:
      return "int_jmp_eq";
    case OpCode::INT_JMP_NEQ:
      return "int_jmp_neq";
    case OpCode::INT_JMP_GT:
      return "int_jmp_gt";
    case OpCode::INT_JMP_GE:
      return "int_jmp_ge";
    case OpCode::INT_JMP_LT:
      return "int_jmp_lt";
    case OpCode::INT_JMP_LE:
      return "int_jmp_le";
    case OpCode::STR_PUSH_CONSTANT:
      return "str_push_constant";
    case OpCode::STR_JMP_EQ:
      return "str_jmp_eq";
    case OpCode::STR_JMP_NEQ:
      return "str_jmp_neq";
    case OpCode::NEW_OBJECT:
      return "new_object";
    case OpCode::PUSH_FROM_OBJECT:
      return "push_from_object";
    case OpCode::POP_INTO_OBJECT:
      return "pop_into_object";
    case OpCode::CALL_INDIRECT:
      return "call_indirect";
    case OpCode::SYSTEM_COLLECT:
      return "system_collect";
    default:
      return "UNKNOWN_BYTECODE";
  }
}

/// Print a OpCode
inline std::ostream &operator<<(std::ostream &out, OpCode bc) {
  return out << toString(bc);
}

using RawInstruction = std::uint32_t;

/// The 24bit immediate encoded in an instruction. Note that parameters are
/// signed values, so special care must be taken to sign extend 24bits to 32.
using Immediate = std::int32_t;

/// A RawInstruction wrapper that will encode and decode instruction opcodes
/// and immediate parameters. The Instruction layout is:
/// ```
/// |0000-0000 0000-0000 0000-0000 0000-0000
/// |---------| opcode (8bits)
///           |-----------------------------| immediate (24bits)
/// ```
///
/// For many ByteCodes, the immediate is unused and left as zero.
class Instruction {
 public:
  constexpr Instruction() noexcept : raw_{0} {}

  constexpr Instruction(RawInstruction raw) noexcept : raw_{raw} {}

  constexpr Instruction(OpCode op) noexcept
      : raw_{RawInstruction(op) << OPCODE_SHIFT} {}

  constexpr Instruction(OpCode op, Immediate p) noexcept
      : raw_{(RawInstruction(op) << OPCODE_SHIFT) | (p & IMMEDIATE_MASK)} {}

  constexpr Instruction &set(OpCode op, Immediate p) noexcept {
    raw_ = (RawInstruction(op) << OPCODE_SHIFT) | (p & IMMEDIATE_MASK);
    return *this;
  }

  /// Encode the opcode
  constexpr Instruction &opCode(OpCode op) noexcept {
    raw_ = (RawInstruction(op) << OPCODE_SHIFT) | (raw_ & IMMEDIATE_MASK);
    return *this;
  }

  /// Decode the opcode
  constexpr OpCode opCode() const noexcept {
    return static_cast<OpCode>(raw_ >> OPCODE_SHIFT);
  }

  /// Encode the immediate
  constexpr Instruction &immediate(Immediate p) noexcept {
    raw_ = (raw_ & OPCODE_MASK) | (p & IMMEDIATE_MASK);
    return *this;
  }

  /// Decode the immediate
  constexpr Immediate immediate() const noexcept {
    auto param = static_cast<Immediate>(raw_ & IMMEDIATE_MASK);
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
  static constexpr const RawInstruction OPCODE_SHIFT = 24;
  static constexpr const RawInstruction IMMEDIATE_MASK = 0x00FF'FFFF;
  static constexpr const RawInstruction OPCODE_MASK = ~IMMEDIATE_MASK;

  RawInstruction raw_;
};

/// A special constant indicating the end of a sequence of instructions.
/// END_SECTION should be the last element in every functions opcode array.
static constexpr Instruction END_SECTION{OpCode::END_SECTION, 0};

/// Print an Instruction.
inline std::ostream &operator<<(std::ostream &out, Instruction i) {
  out << "(" << i.opCode();

  switch (i.opCode()) {
    // 0 parameters
    case OpCode::END_SECTION:
    case OpCode::DUPLICATE:
    case OpCode::FUNCTION_RETURN:
    case OpCode::DROP:
    case OpCode::INT_ADD:
    case OpCode::INT_SUB:
    case OpCode::INT_MUL:
    case OpCode::INT_DIV:
    case OpCode::INT_NOT:
    case OpCode::NEW_OBJECT:
    case OpCode::CALL_INDIRECT:
    case OpCode::SYSTEM_COLLECT:
      break;
    // 1 immediate
    case OpCode::FUNCTION_CALL:
    case OpCode::PRIMITIVE_CALL:
    case OpCode::JMP:
    case OpCode::PUSH_FROM_LOCAL:
    case OpCode::POP_INTO_LOCAL:
    case OpCode::INT_PUSH_CONSTANT:
    case OpCode::INT_JMP_EQ:
    case OpCode::INT_JMP_NEQ:
    case OpCode::INT_JMP_GT:
    case OpCode::INT_JMP_GE:
    case OpCode::INT_JMP_LT:
    case OpCode::INT_JMP_LE:
    case OpCode::STR_PUSH_CONSTANT:
    case OpCode::STR_JMP_EQ:
    case OpCode::STR_JMP_NEQ:
    case OpCode::PUSH_FROM_OBJECT:
    case OpCode::POP_INTO_OBJECT:
    default:
      out << " " << i.immediate();
      break;
  }
  return out << ")";
}

}  // namespace b9

#endif  // B9_INSTRUCTIONS_HPP_
