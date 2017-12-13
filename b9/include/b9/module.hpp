#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/instructions.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <iostream>

namespace b9 {

class Compiler;
class ExecutionContext;
class VirtualMachine;

// Function Definition
struct FunctionDef {
  // Copy Constructor
  FunctionDef(const std::string& name, const std::vector<Instruction> &instructions,
               std::uint32_t nargs = 0, std::uint32_t nregs = 0)
      : instructions{instructions}, nargs{nargs}, nregs{nregs}, name{name} {}
  // Move Constructor
  FunctionDef(const std::string& name, std::vector<Instruction> &&instructions,
               std::uint32_t nargs = 0, std::uint32_t nregs = 0)
      : instructions{std::move(instructions)}, nargs{nargs}, nregs{nregs}, name{name} {}  
  // Function Data
  std::vector<Instruction> instructions;
  std::uint32_t index;
  std::uint32_t nargs;
  std::uint32_t nregs;
  std::string name;
};

inline std::ostream& operator<<(std::ostream& out, const FunctionDef& f) {
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
  std::vector<FunctionDef> functions;
  std::vector<PrimitiveFunction*> primitives;
  std::vector<const char*> strings;

  std::size_t getFunctionIndex(const std::string& name) const {
    for (std::size_t i = 0; i < functions.size(); i++) {
      if (functions[i].name == name) {
        return i;
      }
    }
    throw FunctionNotFoundException{name};
  }

  std::string getFunctionName(const std::size_t index) const {
    if (functions.size() < index) {
      //throw FunctionNotFoundException{index};
      std::cout << "No function at offset " << index << std::endl;
      return NULL;
    }
    return functions[index].name;
  }

};

}  // namespace b9

#endif  // B9_MODULE_HPP_
