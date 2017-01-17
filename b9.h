#ifndef b9_h_
#define b9_h_

#include <cstring>
#include <cstdio>
#include <cstddef>

#include <cstdint>
/* Bytecodes */

/**
 * Each bytecode is 32 bits:
 * first 8 bits are the OpCode
 * second 24 bits may be an argument to the opcode
 */

// runtime options 

// -directcall 0/1 means no jumping to a general purpose 
// intepreter for jit->jit calls
// default == 1
   
// -passparameters 0/1  means to pass parameters 
// to the JIT methods instead of on the interpreter stack
// default == 1

// -operandstack 0/1  toggles to the JITBuilder 
// facility for modelling VM stacks  
// default == 1

typedef uint8_t ByteCode;
typedef int32_t Parameter;  // even though only 24 bits used
typedef uint32_t Instruction;
typedef int64_t StackElement;

#define METHOD_FIRST_BC_OFFSET 0x3
#define NO_MORE_FUNCTIONS      0x0

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

class ExecutionContext
{
public:
    ExecutionContext()
        : stackPointer(this->stack),
         stackEnd(&stack[ (sizeof (stack)/sizeof(StackElement)) - 16])
    {
        std::memset(stack, 0, sizeof(stack)); 
    }

    StackElement stack[1000];
    StackElement* stackPointer;
    StackElement* stackEnd; 
    Instruction **functions;

    /* Command Line Parameters */
    int loopCount = 1;
    int verbose = 0;
    int debug = 0;

    int directCall = 1;
    int passParameters = 1;
    int operandStack = 1;

    char *name = nullptr;

};


typedef StackElement (*Interpret) (ExecutionContext* context, Instruction* program);
 
// define C callable Interpret API for each arg call 
// if args are passed to the function, they are not passed 
// on the intepreter stack
typedef StackElement (*Interpret_0_args) (ExecutionContext* context, Instruction* program);
typedef StackElement (*Interpret_1_args) (ExecutionContext* context, Instruction* program, StackElement p1);
typedef StackElement (*Interpret_2_args) (ExecutionContext* context, Instruction* program, 
    StackElement p1, StackElement p2);
typedef StackElement (*Interpret_3_args) (ExecutionContext* context, Instruction* program,
    StackElement p1, StackElement p2,  StackElement p3);
 

StackElement interpret(ExecutionContext* context, Instruction* program);
void generateCode(Instruction* program, ExecutionContext *context);
void b9_jit_init();

/* Instruction Helpers */

#define decl(argCount, tmpCount) (argCount << 16 | tmpCount)
#define progArgCount(a) (a >> 16)
#define progTmpCount(a) (a & 0xFFFF)


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

// constexpr Instruction
// decl(uint16_t argCount, uint16_t tmpCount)
// {
//     return argCount << 16 | tmpCount;
// }

// constexpr uint16_t
// progArgCount(Instruction a)
// {
//     return a >> 16;
// }

// constexpr uint16_t
// progTmpCount(Instruction a)
// {
//     return a & 0xFFFF;
// }

extern void b9PrintStack(ExecutionContext *context);

#endif
