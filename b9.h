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

#define METHOD_FIRST_BC_OFFSET 0x3

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

typedef int16_t stack_element_t ;

class ExecutionContext {
public:
    ExecutionContext()
        : stackPointer(this->stack),
         stackEnd(&stack[ (sizeof (stack)/sizeof(stack_element_t)) - 16])  // hack for warning
    {
        std::memset(stack, 0, sizeof(stack)); 
    }

    stack_element_t stack[1000];
    stack_element_t* stackPointer;
    stack_element_t* stackEnd;
    Instruction **functions;
};

typedef stack_element_t (*Interpret) (ExecutionContext* context, Instruction* program);
stack_element_t interpret(ExecutionContext* context, Instruction* program);
void generateCode(Instruction* program);
void b9_jit_init();

/* Instruction Helpers */

constexpr ByteCode
getByteCodeFromInstruction(Instruction instruction)
{
    return (instruction >> 16);
}

constexpr Parameter
getParameterFromInstruction(Instruction instruction)
{
    return (instruction & 0xFFFF);
}

constexpr Instruction
createInstruction(ByteCode byteCode, Parameter parameter)
{
    return byteCode << 16 | (parameter & 0xFFFF);
}

#define decl(argCount, tmpCount) (argCount << 16 | tmpCount)
#define progArgCount(a) (a >> 16)
#define progTmpCount(a) (a & 0xFFFF)

extern void b9PrintStack(ExecutionContext *context);
