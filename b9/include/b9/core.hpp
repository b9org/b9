#ifndef b9core_hpp_
#define b9core_hpp_

#include <b9/bytecodes.hpp>
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

class VirtualMachine;
class ExecutionContext;

// Primitive Function from Interpreter call
extern "C" typedef void(PrimitiveFunction)(ExecutionContext *virtualMachine);

void bc_primitive(VirtualMachine *context, Parameter value);

// typedef StackElement (*Interpret) (ExecutionContext* context, Instruction*
// program);

#if defined(B9JIT)

// define C callable Interpret API for each arg call
// if args are passed to the function, they are not passed
// on the intepreter stack

typedef StackElement (*interpret_n_args)(ExecutionContext *context,
                                         Instruction *program, ...);

typedef StackElement (*Interpret_0_args)(ExecutionContext *context,
                                         Instruction *program);
typedef StackElement (*Interpret_1_args)(ExecutionContext *context,
                                         Instruction *program, StackElement p1);
typedef StackElement (*Interpret_2_args)(ExecutionContext *context,
                                         Instruction *program, StackElement p1,
                                         StackElement p2);
typedef StackElement (*Interpret_3_args)(ExecutionContext *context,
                                         Instruction *program, StackElement p1,
                                         StackElement p2, StackElement p3);

typedef StackElement (*JIT_0_args)();
typedef StackElement (*JIT_1_args)(StackElement p1);
typedef StackElement (*JIT_2_args)(StackElement p1, StackElement p2);
typedef StackElement (*JIT_3_args)(StackElement p1, StackElement p2,
                                   StackElement p3);

#endif  // defined(B9JIT)

/* B9 Interpreter */
int parseArguments(ExecutionContext *context, int argc, char *argv[]);

Instruction *getFunctionAddress(ExecutionContext *context,
                                const char *functionName);
// StackElement interpret(ExecutionContext* context, Instruction* program);
StackElement timeFunction(VirtualMachine *virtualMachine, Instruction *function,
                          int loopCount, long *runningTime);

void b9PrintStack(ExecutionContext *context);

bool hasJITAddress(Instruction *p);
void generateCode(ExecutionContext *context, int32_t functionIndex);
void generateAllCode(ExecutionContext *context);
void removeAllGeneratedCode(ExecutionContext *context);
void setJitAddress(ExecutionContext *context, int32_t functionIndex,
                   uint64_t value);

StackElement interpret_0(ExecutionContext *context, Instruction *program);
StackElement interpret_1(ExecutionContext *context, Instruction *program,
                         StackElement p1);
StackElement interpret_2(ExecutionContext *context, Instruction *program,
                         StackElement p1, StackElement p2);
StackElement interpret_3(ExecutionContext *context, Instruction *program,
                         StackElement p1, StackElement p2, StackElement p3);

/* Debug Helpers */
void b9PrintStack(ExecutionContext *context);

const char *b9ByteCodeName(ByteCode bc);

}  // namespace b9
#endif  // b9core_hpp_
