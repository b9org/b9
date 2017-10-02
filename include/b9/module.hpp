#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/core.hpp>
#include <cstdint>
#include <vector>

namespace b9 {

/// Function specification. Metadata about a function.
struct FunctionSpec {
	void* jitAddress;
	Instruction* address;
	uint32_t nargs;
};

/// An interpreter module.
struct Module {
	std::vector<const FunctionSpec*> functions;
	std::vector<PrimitiveFunction*> primitives;
	std::vector<const char*> strings;
};

} // namespace b9

#endif // B9_MODULE_HPP_
