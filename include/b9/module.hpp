#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/core.hpp>
#include <cstdint>
#include <vector>

namespace b9 {

/// Function specification. Metadata about a function.
/// TODO: Indicate tmp space required.
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
/// TODO: Map names->Functions
struct Module {
  std::vector<FunctionSpec> functions;
  std::vector<PrimitiveFunction*> primitives;
  std::vector<const char*> strings;
};

}  // namespace b9

#endif  // B9_MODULE_HPP_
