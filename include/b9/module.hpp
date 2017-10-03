#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

#include <b9/core.hpp>
#include <cstdint>
#include <vector>

namespace b9 {

/// Function specification. Metadata about a function.
/// TODO: Indicate tmp space required.
struct FunctionSpec {
	constexpr FunctionSpec(const Instruction* address, std::uint32_t nargs, void* jitAddress = nullptr)
		: jitAddress{jitAddress}, address{address}, nargs{nargs} {}

	void* jitAddress;
	const Instruction* address;
	std::uint32_t nargs;
};

/// An interpreter module.
/// TODO: Map names->Functions
struct Module {
	std::vector<FunctionSpec> functions;
	std::vector<PrimitiveFunction*> primitives;
	std::vector<const char*> strings;
};

} // namespace b9

#endif // B9_MODULE_HPP_
