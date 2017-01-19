#ifndef b9_h_
#define b9_h_

#include <cstdint>
#include <cstring>

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
#define JMP                 0x9
#define JMP_EQ              0xA
#define JMP_NEQ             0xB
#define JMP_GT              0xC
#define JMP_GE              0xD
#define JMP_LE              0xE
#define JMP_LT              0xF

static constexpr ByteCode
getByteCodeFromInstruction(Instruction instruction)
{
    return (instruction >> 24);
}

static constexpr Parameter
getParameterFromInstruction(Instruction instruction)
{ 
    return instruction & 0x800000 ? instruction | 0xFF000000: instruction & 0xFFFFFF;
}

static constexpr Instruction
createInstruction(ByteCode byteCode, Parameter parameter)
{
    return byteCode << 24 | (parameter & 0xFFFFFF);
}

static constexpr Instruction
decl(uint16_t argCount, uint16_t tmpCount)
{
    return argCount << 16 | tmpCount;
}

static constexpr uint16_t
progArgCount(Instruction a)
{
    return a >> 16;
}

static constexpr uint16_t
progTmpCount(Instruction a)
{
    return a & 0xFFFF;
}

struct ExportedFunctionData {
    const char * name;
    Instruction *program;
    uint64_t jitAddress;
};
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
    struct ExportedFunctionData *functions;

    /* Command Line Parameters */
    int loopCount = 1;
    int verbose = 0;
    int debug = 0;

    int directCall = 1;
    int passParameters = 1;
    int operandStack = 1;

    const char *name = nullptr;
    void *library = nullptr;

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

typedef StackElement (*JIT_0_args)();
typedef StackElement (*JIT_1_args)(StackElement p1);
typedef StackElement (*JIT_2_args)(StackElement p1, StackElement p2);
typedef StackElement (*JIT_3_args)(StackElement p1, StackElement p2, StackElement p3);

/* B9 Interpreter */
void b9_jit_init();
int parseArguments(ExecutionContext *context, int argc, char *argv[]);

bool loadLibrary(ExecutionContext *context, const char *libraryName);
Instruction *getFunctionAddress(ExecutionContext *context, const char *functionName);
StackElement interpret(ExecutionContext* context, Instruction* program);
StackElement timeFunction(ExecutionContext *context, Instruction *function, int loopCount, long* runningTime);

void b9PrintStack(ExecutionContext *context);

bool hasJITAddress(Instruction *p);
void generateCode(ExecutionContext *context, int32_t functionIndex);
void generateAllCode(ExecutionContext *context);
void removeAllGeneratedCode(ExecutionContext *context);
void
setJitAddress(ExecutionContext *context, int32_t functionIndex, uint64_t value);

void push(ExecutionContext *context, StackElement value);
StackElement pop(ExecutionContext *context);


StackElement
interpret_0(ExecutionContext *context, Instruction *program);
StackElement
interpret_1(ExecutionContext *context, Instruction *program, StackElement p1);
StackElement
interpret_2(ExecutionContext *context, Instruction *program, StackElement p1, StackElement p2);
StackElement
interpret_3(ExecutionContext *context, Instruction *program, StackElement p1, StackElement p2, StackElement p3 );

#endif
