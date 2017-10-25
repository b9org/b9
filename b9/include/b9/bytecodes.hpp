#if !defined(B9_BYTECODES_HPP_)
#define B9_BYTECODES_HPP_

#include <cstdint>
#include <ostream>

namespace b9 {

// bits may be an argument to the opcode
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
  // Jump if two integers are equal
  INT_JMP_EQ = 0xe,
  // Jump if two integer are not equal
  INT_JMP_NEQ = 0xf,
  // Jump if the first integer is greater than the second
  INT_JMP_GT = 0x10,
  // Jump if the first integer is greater than or equal to the second
  INT_JMP_GE = 0x11,
  // Jump if the first integer is less than to the second
  INT_JMP_LT = 0x12,
  // Jump if the first integer is less than or equal to the second
  INT_JMP_LE = 0x13,

  // String ByteCodes

  // Push a string from this module's constant pool
  STR_PUSH_CONSTANT = 0x14,
  // Jump if two strings are equal
  STR_JMP_EQ = 0x15,
  // Jump if two strings are not equal
  STR_JMP_NEQ = 0x16,
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

inline std::ostream &operator<<(std::ostream &out, ByteCode bc) {
  return out << toString(bc);
}

}  // namespace b9

#endif  // B9_BYTECODES_HPP_
