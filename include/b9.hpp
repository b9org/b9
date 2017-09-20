#ifndef base9_hpp_
#define base9_hpp_

#include <b9/core.hpp>

#include <string>
#include <cstring>

#undef B9JIT

namespace b9 {

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

class VirtualMachine
{
public:
    VirtualMachine()
      :
      executionContext_{this}
    {
    }

    bool initialize();
    bool shutdown();
    bool parseArguments(int argc, char *argv[]);
    bool loadLibrary();

    Instruction *getFunctionAddress(const char *functionName);
    StackElement runFunction(Instruction *function);

    // private
    Instruction* getFunction(uint64_t index);
    PrimitiveFunction* getPrimitive(uint64_t index);

    const char *getString(int index);

private:
    ExecutionContext executionContext_;

    struct ExportedFunctionData *functions_;
    struct PrimitiveData *primitives_;
    const char ** stringTable_;

    /* Command Line Parameters */
    int loopCount_ = 1;
    int verbose_ = 0;
    int debug_ = 0;

    int directCall_ = 1;
    int passParameters_ = 1;
    int operandStack_ = 1;
    int inlineDepthAllowed_ = 1;

    const char *name_ = nullptr;
    void *library_ = nullptr;
};

} // namespace b9
#endif

