#ifndef B9_VIRTUALMACHINE_HPP_
#define B9_VIRTUALMACHINE_HPP_

#include <b9/instructions.hpp>
#include <b9/module.hpp>
#include <b9/runtime.hpp>

#include <b9/context.inl.hpp>
#include <b9/memorymanager.inl.hpp>
#include <b9/rooting.inl.hpp>

#include <cstring>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace b9 {

class Compiler;
class ExecutionContext;
class VirtualMachine;

struct Config {
  std::size_t maxInlineDepth = 0;  //< The JIT's max inline depth
  bool jit = false;                //< Enable the JIT
  bool directCall = false;         //< Enable direct JIT to JIT calls
  bool passParam = false;          //< Pass arguments in CPU registers
  bool lazyVmState = false;        //< Simulate the VM state
  bool debug = false;              //< Enable debug code
  bool verbose = false;            //< Enable verbose printing and tracing
};

inline std::ostream &operator<<(std::ostream &out, const Config &cfg) {
  out << std::boolalpha;
  out << "Mode:         " << (cfg.jit ? "JIT" : "Interpreter") << std::endl
      << "Inline depth: " << cfg.maxInlineDepth << std::endl
      << "directcall:   " << cfg.directCall << std::endl
      << "passparam:    " << cfg.passParam << std::endl
      << "lazyvmstate:  " << cfg.lazyVmState << std::endl
      << "debug:        " << cfg.debug;
  out << std::noboolalpha;
  return out;
}

struct BadFunctionCallException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

using StackElement = Value;

struct Stack {
  StackElement *stackBase;
  StackElement *stackPointer;
};

typedef RawValue (*JitFunction)(...);

inline bool isReference(StackElement value) {
  return false;  // TODO
}

class ExecutionContext : public RunContext {
 public:
  ExecutionContext(VirtualMachine *virtualMachine, const Config &cfg);

  StackElement interpret(std::size_t functionIndex);

  void push(StackElement value);
  StackElement pop();

  void functionCall(Parameter value);
  void functionReturn(StackElement returnVal);
  void primitiveCall(Parameter value);
  Parameter jmp(Parameter offset);
  void duplicate();
  void drop();
  void pushFromVar(StackElement *args, Parameter offset);
  void pushIntoVar(StackElement *args, Parameter offset);
  void intAdd();
  void intSub();
  // CASCON2017 - Add intMul() and intDiv() here
  void intPushConstant(Parameter value);
  void intNot();
  Parameter intJmpEq(Parameter delta);
  Parameter intJmpNeq(Parameter delta);
  Parameter intJmpGt(Parameter delta);
  Parameter intJmpGe(Parameter delta);
  Parameter intJmpLt(Parameter delta);
  Parameter intJmpLe(Parameter delta);
  void strPushConstant(Parameter value);

  void newObject();

  void pushFromObject(Id slotId);

  void popIntoObject(Id slotId);

  void callIndirect();

  void systemCollect();

  VirtualMachine* virtualMachine() const { return virtualMachine_; }

  void visitStack(Context &cx, Visitor &visitor) {
    const auto n = stackPointer_ - stackBase_;
    std::cout << ">STACK BEGIN" << std::endl;
    for (std::size_t i = 0; i < n; i++) {
      StackElement e = stackBase_[i];
      std::cout << ">STACK[" << i << "] = " << e << std::endl;
      if (e.isPtr()) visitor.rootEdge(cx, this, (Cell*)e.ptr());
    }
    std::cout << ">STACK END" << std::endl;
  }

  // TODO: void strJmpEq(Parameter delta);
  // TODO: void strJmpNeq(Parameter delta);

  // Reset the stack and other internal data
  void reset();

  StackElement *stackBase_;
  StackElement *stackPointer_;

 private:
  // Context cx_;
  StackElement stack_[1000];
  Instruction *programCounter_ = 0;
  StackElement *stackEnd_ = &stack_[1000];
  VirtualMachine *virtualMachine_;
  const Config &cfg_;
};

class VirtualMachine {
 public:
  VirtualMachine(ProcessRuntime &runtime, const Config &cfg);

  ~VirtualMachine() noexcept;

  /// Load a module into the VM.
  void load(std::shared_ptr<const Module> module);
  StackElement run(const std::size_t index,
                   const std::vector<StackElement> &usrArgs);
  StackElement run(const std::string &name,
                   const std::vector<StackElement> &usrArgs);

  const FunctionSpec *getFunction(std::size_t index);
  PrimitiveFunction *getPrimitive(std::size_t index);

  JitFunction getJitAddress(std::size_t functionIndex);
  void setJitAddress(std::size_t functionIndex, JitFunction value);

  std::size_t getFunctionCount();
  JitFunction generateCode(const std::size_t functionIndex);
  void generateAllCode();

  const char *getString(int index);

  ExecutionContext *executionContext() { return &executionContext_; }

  const std::shared_ptr<const Module> &module() { return module_; }

  MemoryManager &memoryManager() { return memoryManager_; }

  const MemoryManager &memoryManager() const { return memoryManager_; }

 private:
  Config cfg_;
  MemoryManager memoryManager_;
  ExecutionContext executionContext_;
  std::shared_ptr<Compiler> compiler_;
  std::shared_ptr<const Module> module_;
  std::vector<JitFunction> compiledFunctions_;
};

typedef StackElement (*Interpret)(ExecutionContext *context,
                                  const std::size_t functionIndex);

// define C callable Interpret API for each arg call
// if args are passed to the function, they are not passed
// on the intepreter stack

StackElement interpret_0(ExecutionContext *context,
                         const std::size_t functionIndex);
StackElement interpret_1(ExecutionContext *context,
                         const std::size_t functionIndex, StackElement p1);
StackElement interpret_2(ExecutionContext *context,
                         const std::size_t functionIndex, StackElement p1,
                         StackElement p2);
StackElement interpret_3(ExecutionContext *context,
                         const std::size_t functionIndex, StackElement p1,
                         StackElement p2, StackElement p3);

void primitive_call(ExecutionContext *context, Parameter value);

inline ExecutionContext::ExecutionContext(VirtualMachine *virtualMachine,
                                          const Config &cfg)
    : RunContext(virtualMachine->memoryManager()),
      stackBase_(this->stack_),
      stackPointer_(this->stack_),
      virtualMachine_(virtualMachine),
      cfg_(cfg) {
  std::memset(stack_, 0, sizeof(StackElement) * 1000);

  userRoots().push_back(
      [this](Context &cx, Visitor &v) { this->visitStack(cx, v); });
}

}  // namespace b9

#endif  // B9_VIRTUALMACHINE_HPP_
