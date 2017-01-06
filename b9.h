// structs from b9
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstddef>

/* Bytecodes */

/**
 * Each bytecode is 32 bytes:
 * first 16 bytes are the OpCode
 * second 16 bytes may be an argument to the opcode
 */

typedef uint16_t ByteCode;
typedef int16_t Parameter;
typedef uint32_t Instruction;

#define NO_MORE_BYTECODES 0x0
#define PUSH_CONSTANT 0X1
#define DROP 0x2
#define PUSH_FROM_VAR 0x3
#define POP_INTO_VAR 4
#define SUB 5
#define ADD 6
#define CALL 7
#define RETURN 8
#define JMPLE 9
#define JMP 10

/* VM State */

class ExecutionContext {
public:
    ExecutionContext()
        : stackPointer(this->stack)
    {
        std::memset(stack, 0, sizeof(stack));
    }

    uint16_t stack[1000];
    uint16_t* stackPointer;
};

uint16_t interpret(ExecutionContext* context, Instruction* program);