#ifndef base9_hpp_
#define base9_hpp_

#include <b9/core.hpp>
#include <b9/callstyle.hpp>

#include <ostream>
#include <string>
#include <cstring>
#include <map>

#undef B9JIT

namespace b9 {

struct JitConfig {
    CallStyle callStyle = CallStyle::interpreter;
    std::size_t maxInlineDepth = 0;
    bool verbose = false;
    bool debug = false;
};

struct VirtualMachineConfig {
    JitConfig jit;
    bool debug = false;
    bool verbose = false;
};

class VirtualMachine;

class ExecutionContext {
 public:
  ExecutionContext(VirtualMachine *virtualMachine)
      : virtualMachine_(virtualMachine)
  {
    std::memset(stack_, 0, sizeof(StackElement) * 1000);
  }

  StackElement interpret(Instruction *program);

  void push(StackElement value);
  StackElement pop();
  void drop();
  void duplicate();
  void functionReturn();
  void functionCall(Parameter value);
  void primitiveCall(Parameter value);
  void pushFromVar(StackElement *args, Parameter offset);
  void pushIntoVar(StackElement *args, Parameter offset);
  void jmp(Parameter offset);

  void intPushConstant(Parameter value);
  void intAdd();
  void intSub();
  Parameter intJmpEq(Parameter delta);
  Parameter intJmpNeq(Parameter delta);
  Parameter intJmpGt(Parameter delta);
  Parameter intJmpGe(Parameter delta);
  Parameter intJmpLt(Parameter delta);
  Parameter intJmpLe(Parameter delta);

  void strPushConstant(Parameter value);

  // Reset the stack and other internal data
  void reset();

 private:
  StackElement stack_[1000];

  StackElement *stackBase_ = stack_;
  StackElement *stackPointer_ = stack_;
  StackElement *stackEnd_ = &stack_[1000];
  Instruction *programCounter_ = 0;

  VirtualMachine *virtualMachine_;
};

class Module {};

using FunctionTable = std::map<const char*, Function*>;

class VirtualMachine
{
public:
    VirtualMachine(const VirtualMachineConfig& cfg)
    : cfg_{cfg},
      executionContext_{this}
    {
    }

    bool initialize();
    bool shutdown();
    bool loadLibrary();
    bool loadLibrary(std::string libraryName);

    Instruction *getFunctionAddress(const char *functionName);
    StackElement runFunction(Instruction *function);

    // private
    Instruction* getFunction(uint64_t index);
    PrimitiveFunction* getPrimitive(uint64_t index);

    const char *getString(int index);


private:
    VirtualMachineConfig cfg_;
    ExecutionContext executionContext_;
    FunctionTable functionTable_;
    ExportedFunctionData* functions_;
    PrimitiveData* primitives_;
    const char** stringTable_;
};

} // namespace b9

#endif // 

