#if !defined(B9_BYTECODES_HPP_)
#define B9_BYTECODES_HPP_

#include <cstdint>

namespace b9 {

#define NO_MORE_BYTECODES 0x0

// ByteCodes

// Each Intruction is 32 bits: The first 8 bits are the ByteCode The second 24
// bits may be an argument to the opcode

using ByteCodeSize = std::uint8_t;
using Parameter = std::int32_t;  // even though only 24 bits used
using Instruction = std::uint32_t;
using StackElement = std::int64_t;

enum class ByteCode : ByteCodeSize {
    // Generic ByteCodes

    // Drop the top element of the stack
    drop = 0x1,
    // Duplicate the top element on the stack
    duplicate = 0x2,
    // Return from a function
    functionReturn = 0x3,
    // Call a Base9 function
    functionCall  = 0x4,
    // Call a native C function
    primitiveCall = 0x5,
    // Jump unconditionally by the offset
    jmp = 0x6,
    // Push from a local variable
    pushFromVar = 0x7,
    // Push into a local variable
    popIntoVar = 0x8,

    // Integer bytecodes

    // Push a constant
    intPushConstant = 0x9,
    // Subtract two integers
    intSub = 0xa,
    // Add two integers
    intAdd = 0xb,
    // Jump if two integers are equal
    intJmpEq = 0xc,
    // Jump if two integer are not equal
    intJmpNeq = 0xd,
    // Jump if the first integer is greater than the second
    intJmpGt = 0xe,
    // Jump if the first integer is greater than or equal to the second
    intJmpGe = 0xf,
    // Jump if the first integer is less than to the second
    intJmpLt = 0x10,
    // Jump if the first integer is less than or equal to the second
    intJmpLe = 0x11,

    // String ByteCodes

    // Push a string from this module's constant pool
    strPushConstant = 0x12,
    // Jump if two strings are equal
    strJmpEq = 0x13,
    // Jump if two integer are not equal
    strJmpNeq = 0x14,
};

} // namespace b9

#endif // B9_BYTECODES_HPP_
