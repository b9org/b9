#if !defined(B9_BYTECODES_HPP_)
#define B9_BYTECODES_HPP_

#include <cstdint>
#include <ostream>

namespace b9 {

#define NO_MORE_BYTECODES 0x0

// bits may be an argument to the opcode
using RawByteCode = std::uint8_t;

enum class ByteCode : RawByteCode {
  // Generic ByteCodes

  // Drop the top element of the stack
  DROP = 0x1,
  // Duplicate the top element on the stack
  DUPLICATE = 0x2,
  // Return from a function
  FUNCTION_RETURN = 0x3,
  // Call a Base9 function
  FUNCTION_CALL = 0x4,
  // Call a native C function
  PRIMITIVE_CALL = 0x5,
  // Jump unconditionally by the offset
  JMP = 0x6,
  // Push from a local variable
  PUSH_FROM_VAR = 0x7,
  // Push into a local variable
  POP_INTO_VAR = 0x8,

  // Integer bytecodes

  // Push a constant
  INT_PUSH_CONSTANT = 0x9,
  // Subtract two integers
  INT_SUB = 0xa,
  // Add two integers
  INT_ADD = 0xb,
  // Jump if two integers are equal
  INT_JMP_EQ = 0xc,
  // Jump if two integer are not equal
  INT_JMP_NEQ = 0xd,
  // Jump if the first integer is greater than the second
  INT_JMP_GT = 0xe,
  // Jump if the first integer is greater than or equal to the second
  INT_JMP_GE = 0xf,
  // Jump if the first integer is less than to the second
  INT_JMP_LT = 0x10,
  // Jump if the first integer is less than or equal to the second
  INT_JMP_LE = 0x11,

  // String ByteCodes

  // Push a string from this module's constant pool
  STR_PUSH_CONSTANT = 0x12,
  // Jump if two strings are equal
  STR_JMP_EQ = 0x13,
  // Jump if two integer are not equal
  STR_JMP_NEQ = 0x14,
};

inline const char *toString(ByteCode bc) {
  switch (bc) {
    case ByteCode::DROP:
      return "DROP";
    case ByteCode::DUPLICATE:
      return "DUPLICATE";
    case ByteCode::FUNCTION_RETURN:
      return "FUNCTION_RETURN";
    case ByteCode::FUNCTION_CALL:
      return "FUNCTION_CALL";
    case ByteCode::PRIMITIVE_CALL:
      return "PRIMITIVE_CALL";
    case ByteCode::JMP:
      return "JMP";
    case ByteCode::PUSH_FROM_VAR:
      return "PUSH_FROM_VAR";
    case ByteCode::POP_INTO_VAR:
      return "POP_INTO_VAR";
    case ByteCode::INT_PUSH_CONSTANT:
      return "INT_PUSH_CONSTANT";
    case ByteCode::INT_SUB:
      return "INT_SUB";
    case ByteCode::INT_ADD:
      return "INT_ADD";
    case ByteCode::INT_JMP_EQ:
      return "INT_JMP_EQ";
    case ByteCode::INT_JMP_NEQ:
      return "INT_JMP_NEQ";
    case ByteCode::INT_JMP_GT:
      return "INT_JMP_GT";
    case ByteCode::INT_JMP_GE:
      return "INT_JMP_GE";
    case ByteCode::INT_JMP_LT:
      return "INT_JMP_LT";
    case ByteCode::INT_JMP_LE:
      return "INT_JMP_LE";
    case ByteCode::STR_PUSH_CONSTANT:
      return "STR_PUSH_CONSTANT";
    case ByteCode::STR_JMP_EQ:
      return "STR_JMP_EQ";
    case ByteCode::STR_JMP_NEQ:
      return "STR_JMP_NEQ";
    default:
      return "UNKNOWN BYTECODE";
  }
}

inline std::ostream &operator<<(std::ostream &out, ByteCode bc) {
  out << std::hex;
  return out << toString(bc) << " [" << (RawByteCode)bc << "]";
  out << std::dec;
}

}  // namespace b9

#endif  // B9_BYTECODES_HPP_
