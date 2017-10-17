#ifndef B9_HPP_
#define B9_HPP_

#include <b9/bytecodes.hpp>
#include <b9/callstyle.hpp>
#include <b9/core.hpp>
#include <b9/jit.hpp>
#include <b9/module.hpp>

#include <cstring>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace b9 {

struct JitConfig {
  CallStyle callStyle = CallStyle::interpreter;
  std::size_t maxInlineDepth = 0;
  bool operandStack = true;
  bool verbose = false;
  bool debug = false;
};

struct VirtualMachineConfig {
  JitConfig jitConfig;
  bool jitEnabled;
  bool debug = false;
  bool verbose = false;
};

class VirtualMachine;

struct BadFunctionCallException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct Stack {
  StackElement *stackBase;
  StackElement *stackPointer;
};

class ExecutionContext {
 public:
  ExecutionContext(VirtualMachine *virtualMachine)
      : virtualMachine_(virtualMachine) {
    std::memset(stack_, 0, sizeof(StackElement) * 1000);
  }

  StackElement interpret(const FunctionSpec *function);

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

  // private
  Stack stackFields = {stack_, stack_};

 private:
  StackElement stack_[1000];
  StackElement *&stackBase_ = stackFields.stackBase;
  StackElement *&stackPointer_ = stackFields.stackPointer;
  Instruction *programCounter_ = 0;
  StackElement *stackEnd_ = &stack_[1000];
  VirtualMachine *virtualMachine_;
};

class VirtualMachine {
 public:
  VirtualMachine(const VirtualMachineConfig &cfg)
      : cfg_{cfg}, executionContext_{this}, compiler_{nullptr} {}

  bool initialize();
  bool shutdown();

  /// Load a module into the VM.
  void load(std::shared_ptr<const Module> module);
  StackElement run(const std::size_t index,
                   const std::vector<StackElement> &usrArgs);
  StackElement run(const std::string &name,
                   const std::vector<StackElement> &usrArgs);

  // private
  const FunctionSpec *getFunction(std::size_t index);
  PrimitiveFunction *getPrimitive(std::size_t index);

  void *getJitAddress(std::size_t functionIndex);
  void setJitAddress(std::size_t functionIndex, void *value);

  std::size_t getFunctionCount();
  void generateCode(int32_t functionIndex);
  void generateAllCode();

  const char *getString(int index);

 private:
  VirtualMachineConfig cfg_;
  ExecutionContext executionContext_;
  Compiler *compiler_;
  std::shared_ptr<const Module> module_;

  std::vector<void *> compiledFunctions_;
};

}  // namespace b9

#endif  // B9_HPP_
