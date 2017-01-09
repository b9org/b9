// structs from b9
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstddef>

/* Bytecodes */

/**
 * Each bytecode is 32 bits:
 * first 8 bits are the OpCode
 * second 24 bits may be an argument to the opcode
 */

typedef uint8_t ByteCode;
typedef int32_t Parameter;  // even though only 24 bits used
typedef uint32_t Instruction;

#define METHOD_FIRST_BC_OFFSET 0x3

#define NO_MORE_BYTECODES   0x0
#define PUSH_CONSTANT       0x1
#define DROP                0x2
#define PUSH_FROM_VAR       0x3
#define POP_INTO_VAR        0x4
#define SUB                 0x5
#define ADD                 0x6
#define CALL                0x7
#define RETURN              0x8
#define JMPLE               0x9
#define JMP                 0xA

/* VM State */

typedef int64_t stack_element_t ;

class ExecutionContext {
public:
    ExecutionContext()
        : stackPointer(this->stack),
         stackEnd(&stack[ (sizeof (stack)/sizeof(stack_element_t)) - 16])
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
    return (instruction >> 24);
}

constexpr Parameter
getParameterFromInstruction(Instruction instruction)
{ 
    return instruction & 0x800000 ? instruction | 0xFF000000: instruction & 0xFFFFFF;
}

constexpr Instruction
createInstruction(ByteCode byteCode, Parameter parameter)
{
    return byteCode << 24 | (parameter & 0xFFFFFF);
}

#define decl(argCount, tmpCount) (argCount << 16 | tmpCount)
#define progArgCount(a) (a >> 16)
#define progTmpCount(a) (a & 0xFFFF)

extern void b9PrintStack(ExecutionContext *context);
