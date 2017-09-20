#ifndef b9core_hpp_
#define b9core_hpp_

#include <string>
#include <cstdint>

namespace b9 {

#define NO_MORE_BYTECODES 0x0

// ByteCodes

// Each Intruction is 32 bits: The first 8 bits are the ByteCode The second 24
// bits may be an argument to the opcode

typedef uint8_t ByteCodeSize;
typedef int32_t Parameter;  // even though only 24 bits used
typedef uint32_t Instruction;
typedef int64_t StackElement;

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

class ByteCodes final {
 public:
  static ByteCode fromByte(uint8_t byte) { return static_cast<ByteCode>(byte); }
  static uint8_t toByte(ByteCode byteCode) { return static_cast<uint8_t>(byteCode); }

};

class Instructions final {
public:
    static Instruction create(ByteCode byteCode, Parameter parameter) {
        uint8_t byte = ByteCodes::toByte(byteCode);
        return byte << 24 | (parameter & 0xFFFFFF);
    }

    static ByteCode getByteCode (Instruction instruction) {
        return static_cast<ByteCode>(instruction >> 24);
    }

    static Parameter getParameter(Instruction instruction) {
        return static_cast<Parameter>(instruction & 0x800000 ? instruction | 0xFF000000: instruction & 0xFFFFFF);
    }
};

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

class VirtualMachine;
class ExecutionContext;

// Primitive Function from Interpreter call
extern "C" typedef void (PrimitiveFunction)(VirtualMachine *virtualMachine);

void
bc_primitive(VirtualMachine *context, Parameter value);

struct PrimitiveData {
    const char * name;
    PrimitiveFunction *address;
};

class FunctionSpecification {
public:
    const char* name_ ;
    uint32_t nargs_;
    Instruction *byteCodes_;
    uint64_t jitAddress_;
};

class ExportedFunctionData {
public:
    int functionCount_;
    FunctionSpecification *functionTable_;
};

//typedef StackElement (*Interpret) (ExecutionContext* context, Instruction* program);

#if defined(B9JIT)

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

#endif // defined(B9JIT)

/* B9 Interpreter */
int parseArguments(ExecutionContext *context, int argc, char *argv[]);

Instruction *getFunctionAddress(ExecutionContext *context, const char *functionName);
//StackElement interpret(ExecutionContext* context, Instruction* program);
StackElement timeFunction(VirtualMachine *virtualMachine, Instruction *function, int loopCount, long* runningTime);

void b9PrintStack(ExecutionContext *context);

bool hasJITAddress(Instruction *p);
void generateCode(ExecutionContext *context, int32_t functionIndex);
void generateAllCode(ExecutionContext *context);
void removeAllGeneratedCode(ExecutionContext *context);
void
setJitAddress(ExecutionContext *context, int32_t functionIndex, uint64_t value);

StackElement
interpret_0(ExecutionContext *context, Instruction *program);
StackElement
interpret_1(ExecutionContext *context, Instruction *program, StackElement p1);
StackElement
interpret_2(ExecutionContext *context, Instruction *program, StackElement p1, StackElement p2);
StackElement
interpret_3(ExecutionContext *context, Instruction *program, StackElement p1, StackElement p2, StackElement p3 );


/* Debug Helpers */
void
b9PrintStack(ExecutionContext *context);

const char *
b9ByteCodeName(ByteCode bc);

} // namespace b9
#endif // b9core_hpp_
