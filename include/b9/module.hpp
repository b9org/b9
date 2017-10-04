#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/core.hpp>
#include <cstdint>
#include <vector>
#include <stdexcept>

namespace b9 {

/// Function specification. Metadata about a function.
struct FunctionSpec {
  FunctionSpec(const std::string& name, const Instruction* address,
               std::uint32_t nargs = 0, std::uint32_t nregs = 0,
               void* jitAddress = nullptr)
      : jitAddress{jitAddress},
        address{address},
        nargs{nargs},
        nregs{nregs},
        name{name} {}

  void* jitAddress;
  const Instruction* address;
  std::uint32_t nargs;
  std::uint32_t nregs;
  std::string name;
};

/// An interpreter module.
struct Module {
  std::vector<FunctionSpec> functions;
  std::vector<PrimitiveFunction*> primitives;
  std::vector<const char*> strings;

  // Arianne
  int findFunction(const std::string& name) const {
    for (int i = 0; i < functions.size(); i++) {
      if (functions[i].name == name) {
        return i;
      }
    }
    throw std::invalid_argument{"Function argument not found" };
  }
};

}  // namespace b9

#endif  // B9_MODULE_HPP_
