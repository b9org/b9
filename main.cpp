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
typedef uint16_t Parameter;
typedef uint32_t Instruction;

#define PUSH_CONSTANT 0X1
#define DROP 0x2
#define PUSH_FROM_VAR 0x3
#define POP_INTO_VAR 4
#define SUB 5
#define ADD 6
#define CALL 7
#define RETURN 8
#define JMPLE 9

constexpr ByteCode
getByteCodeFromInstruction (Instruction instruction)
{
    return (instruction >> 16);
}

constexpr Parameter
getParameterFromInstruction (Instruction instruction)
{
    return (instruction & 0xFFFF);
}

constexpr Instruction
createInstruction(ByteCode byteCode, Parameter parameter)
{
    return byteCode << 16 | parameter;
}

#define decl(argCount, tmpCount, length) (argCount << 24 | tmpCount << 16 | length)
#define progArgCount(a) (a >> 24)
#define progTmpCount(a) (a >> 16 & 0xF)
#define progLength(a) (a & 0xFFFF)


/* Byte Code Implementations */

void push(ByteCode **programPointer, uint16_t **stackPointer, uint16_t value)
{

}

void
bc_push (Instruction **programPointer, uint16_t **stackPointer, uint16_t value)
{
    **stackPointer = value;
    ++(*stackPointer);
    ++(*programPointer);
}

void
bc_jmple(Instruction **programPointer, uint16_t **stackPointer, int16_t delta)
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
bc_add (Instruction **programPointer, uint16_t **stackPointer)
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
    Instruction *instructionPointer;
    uint16_t *stackPointer;

    uint16_t interpret(Instruction *program)
    {
        int args = progArgCount(*program);
        int tmps = progTmpCount(*program);
        int length = progLength(*program);

        printf("Prog Arg Count %d, tmp count %d, length %d", args, tmps, length);

        instructionPointer = program + 1;

        while (instructionPointer < program + length)
        {
            switch (getByteCodeFromInstruction(*instructionPointer))
            {
                case PUSH_CONSTANT :
                    printf("push\n");
                    bc_push(&this->instructionPointer, &this->stackPointer, getParameterFromInstruction(*instructionPointer));
                    break;
                case ADD :
                    printf("add\n");
                    bc_add(&this->instructionPointer, &this->stackPointer);
                    break;
            }
        }
        return *(stackPointer-1);
    }
};


static Instruction fib_function [] = {
 decl(0, 0, 2), //sizeof(fib_function) / sizeof(Instruction)),
 createInstruction(PUSH_CONSTANT, 9),
 createInstruction(RETURN, 0)
};

static Instruction main_function [] =
{
    decl(0, 0, 4), //sizeof(main_function) / sizeof(Instruction)),
    createInstruction(PUSH_CONSTANT, 1),
    createInstruction(PUSH_CONSTANT, 5),
    createInstruction(ADD, 0),
    createInstruction(RETURN, 0)
};

/* Byte Code Program */
static Instruction *functions [] = {
    main_function,
    fib_function
};

/* Main Loop */

int
main ()
{
    Instruction *programToRun = main_function;
    ByteCodeInterpreter byteCodeInterpreter;

    uint16_t result = byteCodeInterpreter.interpret(programToRun);

    printf("Program result is: %d", result);

    return 0;
}
