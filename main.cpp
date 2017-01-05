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

typedef uint16_t OpCode;
typedef uint32_t ByteCode;

enum BC_OPCODES : OpCode {
    PUSH_CONSTANT     = 0x1,
    DROP
    PUSH_FROM_VAR
    POP_INTO_VAR
    SUB
    ADD      = 0x3,
    CALL
    RETURN
    JMPLE
};


class BYTECODE {
    public:
        static constexpr OpCode getOpCodeFromByteCode (ByteCode byteCode) { return *(OpCode *)&byteCode; }
        static constexpr uint16_t getParamFromByteCode (ByteCode byteCode) { return *(((uint16_t *)&byteCode)+1);}

        static constexpr ByteCode createByteCode(const OpCode opCode, const uint16_t param)
        {
            uint16_t byteCode [2] = {opCode, param};
            return *(ByteCode *) byteCode;
        }
};

class BYTECODE_PUSH : BYTECODE {
    public:
        static constexpr OpCode getOpCode() { return (OpCode) BC_OPCODES::PUSH; }

        static constexpr uint16_t getValue(ByteCode byteCode) { return getParamFromByteCode(byteCode); }
        static constexpr ByteCode gen(const uint16_t param) { return BYTECODE::createByteCode(BYTECODE_PUSH::getOpCode(), param); }

};

class BYTECODE_ADD : BYTECODE {
    public:
        static constexpr OpCode getOpCode() { return (OpCode) BC_OPCODES::ADD; }
        static constexpr ByteCode gen() { return BYTECODE::createByteCode(BYTECODE_ADD::getOpCode(), 0); }
};

/* Byte Code Implementations */

void
bc_push (ByteCode **programPointer, uint16_t **stackPointer, uint16_t value)
{
    **stackPointer = value;
    ++(*stackPointer);
    ++(*programPointer);
}

void
bc_jmple(ByteCode **programPointer, uint16_t **stackPointer, int16_t delta)
{

    --(*stackPointer);
    uint16_t operandA = **stackPointer;

    --(*stackPointer);
    uint16_t operandB = **stackPointer;

    if (operandB < operandA) {
        *programPointer += delta;
    } else {
        ++(*programPointer);
    }
}


void
bc_add (ByteCode **programPointer, uint16_t **stackPointer)
{
    --(*stackPointer);
    uint16_t operandA = **stackPointer;

    --(*stackPointer);
    uint16_t operandB = **stackPointer;

    uint16_t result = operandA + operandB;

    **stackPointer = result;
    ++(*stackPointer);

    ++(*programPointer);
}

/* ByteCode Interpreter */

class ByteCodeInterpreter
{
public:
    ByteCodeInterpreter () :
        stackPointer(this->stack)
    {
        std::memset(stack, 0, sizeof(stack));
    }

    uint16_t stack[1000];
    ByteCode *programPointer;
    uint16_t *stackPointer;

    uint16_t interpret(ByteCode *program, std::size_t programSize)
    {
        programPointer = program;
        while (programPointer < program + programSize)
        {
            switch (BYTECODE::getOpCodeFromByteCode(*programPointer))
            {
                case static_cast<unsigned int>(BC_OPCODES::PUSH) :
                    printf("push\n");
                    bc_push(&this->programPointer, &this->stackPointer, BYTECODE_PUSH::getValue(*programPointer));
                    break;
                case static_cast<unsigned int>(BC_OPCODES::ADD) :
                    printf("add\n");
                    bc_add(&this->programPointer, &this->stackPointer);
                    break;
            }
        }
        return *(stackPointer-1);
    }
};


static ByteCode fib_function [] = {};

static ByteCode main_function [] =
{
    BYTECODE_PUSH::gen(1),
    BYTECODE_PUSH::gen(2),
    BYTECODE_ADD::gen(),
    BYTECODE_PUSH::gen(0),
    BYTECODE_PUSH::gen(2),
    BYTECODE_JMPLE::gen(2),
};

/* Byte Code Program */
static ByteCode *functions [] = {
    &main_function,
    &fib_function
}

static ByteCode program_size = 3;

/* Main Loop */

int
main ()
{
    ByteCode *programToRun = program;
    std::size_t byteCodeCount = program_size;

    ByteCodeInterpreter byteCodeInterpreter;

    uint16_t result = byteCodeInterpreter.interpret(programToRun, byteCodeCount);

    printf("Program result is: %d", result);

    return 0;
}
