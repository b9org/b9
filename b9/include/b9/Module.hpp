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
  std::string name;
  std::vector<Instruction> instructions;
  std::uint32_t nparams;
  std::uint32_t nlocals;
};

inline void operator<<(std::ostream& out, const FunctionDef& f) {
  out << "(function \"" << f.name << "\" " << f.nparams << " " << f.nlocals;
  std::size_t i = 0;
  for (auto instruction : f.instructions) {
    out << std::endl << "  " << i << "  " << instruction;
    i++;
  }
  out << ")" << std::endl << std::endl;
}

inline bool operator==(const FunctionDef& lhs, const FunctionDef& rhs) {
  return lhs.name == rhs.name && lhs.nparams == rhs.nparams &&
         lhs.nlocals == rhs.nlocals;
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
  for (auto string : m.strings) {
    out << "(string \"" << string << "\")" << std::endl;
  }
  out << std::endl;
}

inline bool operator==(const Module& lhs, const Module& rhs) {
  return lhs.functions == rhs.functions && lhs.strings == rhs.strings;
}

}  // namespace b9

#endif  // B9_MODULE_HPP_
