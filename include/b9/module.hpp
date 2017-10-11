#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/core.hpp>
#include <cstdint>
#include <vector>

namespace b9 {

/// Function specification. Metadata about a function.
struct FunctionSpec {
  FunctionSpec(const std::string& name, const Instruction* address,
               std::uint32_t nargs = 0, std::uint32_t nregs = 0)
      : address{address},
        nargs{nargs},
        nregs{nregs},
        name{name} {}

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
};

}  // namespace b9

#endif  // B9_MODULE_HPP_