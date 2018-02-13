#ifndef B9_VIRTUALMACHINE_HPP_
#define B9_VIRTUALMACHINE_HPP_

#include <b9/OperandStack.hpp>
#include <b9/compiler/Compiler.hpp>
#include <b9/instructions.hpp>
#include <b9/module.hpp>

#include <OMR/Om/Allocator.inl.hpp>
#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/Map.inl.hpp>
#include <OMR/Om/MemoryManager.inl.hpp>
#include <OMR/Om/Object.inl.hpp>
#include <OMR/Om/ObjectMap.inl.hpp>
#include <OMR/Om/RootRef.inl.hpp>
#include <OMR/Om/Runtime.hpp>
#include <OMR/Om/TransitionSet.inl.hpp>
#include <OMR/Om/Traverse.hpp>
#include <OMR/Om/Value.hpp>

#include <cstring>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

extern "C" {
b9::PrimitiveFunction b9_prim_print_string;
b9::PrimitiveFunction b9_prim_print_number;
}

namespace b9 {

namespace Om = ::OMR::Om;

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

typedef Om::Value (*JitFunction)(ExecutionContext *executionContext, ...);

class VirtualMachine {
 public:
  VirtualMachine(OMR::Om::ProcessRuntime &runtime, const Config &cfg);

  ~VirtualMachine() noexcept;

  /// Load a module into the VM.
  void load(std::shared_ptr<const Module> module);

  StackElement run(const std::size_t index,
                   const std::vector<StackElement> &usrArgs);

  StackElement run(const std::string &name,
                   const std::vector<StackElement> &usrArgs);

  const FunctionDef *getFunction(std::size_t index);

  PrimitiveFunction *getPrimitive(std::size_t index);

  JitFunction getJitAddress(std::size_t functionIndex);

  void setJitAddress(std::size_t functionIndex, JitFunction value);

  std::size_t getFunctionCount();

  JitFunction generateCode(const std::size_t functionIndex);

  void generateAllCode();

  const std::string& getString(int index);

  const std::shared_ptr<const Module> &module() { return module_; }

  OMR::Om::MemoryManager &memoryManager() { return memoryManager_; }

  const OMR::Om::MemoryManager &memoryManager() const { return memoryManager_; }

  std::shared_ptr<Compiler> compiler() { return compiler_; }

  const Config &config() { return cfg_; }

 private:
  static constexpr PrimitiveFunction *const primitives_[] = {
      b9_prim_print_string, b9_prim_print_number};

  Config cfg_;
  OMR::Om::MemoryManager memoryManager_;
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

}  // namespace b9

#endif  // B9_VIRTUALMACHINE_HPP_
