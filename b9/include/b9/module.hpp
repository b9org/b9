#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/instructions.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace b9 {

class Compiler;
class ExecutionContext;
class VirtualMachine;

/// Function specification copy constructor. Metadata about a function.
struct FunctionSpec {
  FunctionSpec(const std::string& name, const std::vector<Instruction> &instructions,
               std::uint32_t nargs = 0, std::uint32_t nregs = 0)
      : instructions{instructions}, nargs{nargs}, nregs{nregs}, name{name} {}

  FunctionSpec(const std::string& name, std::vector<Instruction> &&instructions,
               std::uint32_t nargs = 0, std::uint32_t nregs = 0)
      : instructions{std::move(instructions)}, nargs{nargs}, nregs{nregs}, name{name} {}  

  std::vector<Instruction> instructions;
  std::uint32_t nargs;
  std::uint32_t nregs;
  std::string name;
};

inline std::ostream& operator<<(std::ostream& out, const FunctionSpec& f) {
  return out << f.name << " (" << &(f.instructions) << "): {nargs: " << f.nargs
             << ", nregs: " << f.nregs << "}";
}

// Primitive Function from Interpreter call
extern "C" typedef void(PrimitiveFunction)(ExecutionContext* virtualMachine);

/// Function not found exception.
struct FunctionNotFoundException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/// An interpreter module.
struct Module {
  std::vector<FunctionSpec> functions;
  std::vector<PrimitiveFunction*> primitives;
  std::vector<const char*> strings;

  std::size_t findFunction(const std::string& name) const {
    for (std::size_t i = 0; i < functions.size(); i++) {
      if (functions[i].name == name) {
        return i;
      }
    }
    throw FunctionNotFoundException{name};
  }
};

}  // namespace b9

#endif  // B9_MODULE_HPP_
