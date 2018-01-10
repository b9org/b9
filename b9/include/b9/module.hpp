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
  std::vector<std::string> strings;

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
      throw FunctionNotFoundException{std::to_string(index)};
    }
    return functions[index].name;
  }

	void printModule() {
		int32_t index = 0;
		for (auto it = functions.begin(); it != functions.end();
				 it++) {
			std::cout << "Function Data at index " << index << ": " << std::endl
								<< "   Name: " << it->name << ", Number Arguments: " << it->nargs
								<< ", Number Registers: " << it->nregs << std::endl;
			std::cout << "   Instructions: " << std::endl;
			for (auto instruction : it->instructions) {
				std::cout << std::hex;
				std::cout << "      " << instruction << std::endl;
				std::cout << std::dec;
			}
			++index;
		}
		std::cout << "String Table:" << std::endl;
		for (auto string : strings) {
			std::cout << "   " << string << std::endl;
		}
	}

};

}  // namespace b9

#endif  // B9_MODULE_HPP_
