#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/instructions.hpp>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace b9 {

class Compiler;
class ExecutionContext;
class VirtualMachine;

// Function Definition
struct FunctionDef {
  // Copy Constructor
  FunctionDef(const std::string& name, std::uint32_t index,
              const std::vector<Instruction>& instructions,
              std::uint32_t nargs = 0, std::uint32_t nregs = 0)
      : name{name},
        index{index},
        instructions{instructions},
        nargs{nargs},
        nregs{nregs} {}

  // Move Constructor
  FunctionDef(const std::string& name, std::uint32_t index,
              std::vector<Instruction>&& instructions, std::uint32_t nargs = 0,
              std::uint32_t nregs = 0)
      : name{name},
        index{index},
        instructions{std::move(instructions)},
        nargs{nargs},
        nregs{nregs} {}

  // Function Data
  std::string name;
  uint32_t index;
  std::uint32_t nargs;
  std::uint32_t nregs;
  std::vector<Instruction> instructions;
};

inline void operator<<(std::ostream& out, const FunctionDef& f) {
  out << "Function Name: " << f.name << ", Index: " << f.index << std::endl
      << "   Number Arguments: " << f.nargs << ", Number Registers: " << f.nregs
      << std::endl
      << "   Instructions: " << std::endl;
  for (auto instruction : f.instructions) {
    out << "      " << instruction << std::endl;
  }
  out << std::endl;
}

inline bool operator==(const FunctionDef& lhs, const FunctionDef& rhs) {
  return lhs.name == rhs.name && lhs.index == rhs.index &&
         lhs.nargs == rhs.nargs && lhs.nregs == rhs.nregs;
}

/// Function not found exception.
struct FunctionNotFoundException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

// Primitive Function from Interpreter call
extern "C" typedef void(PrimitiveFunction)(ExecutionContext* context);

/// An interpreter module.
struct Module {
  std::vector<FunctionDef> functions;
  std::vector<PrimitiveFunction*> primitives;
  std::vector<std::string> strings;

  std::size_t getFunctionIndex(const std::string& name) const {
    for (std::size_t i = 0; i < functions.size(); i++) {
      if (functions[i].name == name) {
        return i;
      }
    }
    throw FunctionNotFoundException{name};
  }
};

inline void operator<<(std::ostream& out, const Module& m) {
  int32_t index = 0;
  for (auto function : m.functions) {
    out << function;
    ++index;
  }
  out << "String Table:" << std::endl;
  for (auto string : m.strings) {
    out << "   " << string << std::endl;
  }
  out << std::endl;
}

inline bool operator==(const Module& lhs, const Module& rhs) {
  return lhs.functions == rhs.functions && lhs.strings == rhs.strings &&
         lhs.primitives == rhs.primitives;
}

}  // namespace b9

#endif  // B9_MODULE_HPP_
