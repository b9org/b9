#if !defined(B9_COMPILER_HPP_)
#define B9_COMPILER_HPP_

#include "b9/compiler/GlobalTypes.hpp"
#include "b9/instructions.hpp"

#include <Jit.hpp>
#include <ilgen/TypeDictionary.hpp>

#include <OMR/Om/Value.hpp>

#include <vector>

namespace b9 {

class Config;
class FunctionSpec;
class Stack;
class VirtualMachine;
class ExecutionContext;

typedef OMR::Om::RawValue (*JitFunction)(ExecutionContext* executionContext, ...);

/// Function not found exception.
struct CompilationException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class Compiler {
 public:
  Compiler(VirtualMachine &virtualMachine, const Config &cfg);
  JitFunction generateCode(const std::size_t functionIndex);

  const GlobalTypes &globalTypes() const { return globalTypes_; }

  TR::TypeDictionary &typeDictionary() { return typeDictionary_; }

  const TR::TypeDictionary &typeDictionary() const { return typeDictionary_; }

 private:
  TR::TypeDictionary typeDictionary_;
  const GlobalTypes globalTypes_;
  VirtualMachine &virtualMachine_;
  const Config &cfg_;
};

}  // namespace b9

#endif  // B9_COMPILER_HPP_
