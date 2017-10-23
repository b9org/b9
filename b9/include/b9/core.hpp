#ifndef b9core_hpp_
#define b9core_hpp_

#include <b9/bytecodes.hpp>
#include <b9/instruction.hpp>
#include <b9/interpreter.hpp>

#include <cstdint>
#include <string>

namespace b9 {

class ByteCodes final {
 public:
  static ByteCode fromByte(uint8_t byte) { return static_cast<ByteCode>(byte); }
  static uint8_t toByte(ByteCode byteCode) {
    return static_cast<uint8_t>(byteCode);
  }
};

class Instructions final {
 public:
  static Instruction create(ByteCode byteCode, Parameter parameter) {
    uint8_t byte = ByteCodes::toByte(byteCode);
    return byte << 24 | (parameter & 0xFFFFFF);
  }

  static ByteCode getByteCode(Instruction instruction) {
    return static_cast<ByteCode>(instruction >> 24);
  }

  static Parameter getParameter(Instruction instruction) {
    return static_cast<Parameter>(instruction & 0x800000
                                      ? instruction | 0xFF000000
                                      : instruction & 0xFFFFFF);
  }
};

}  // namespace b9
#endif  // b9core_hpp_
