#if !defined(B9_MODULE_HPP_)
#define B9_MODULE_HPP_

namespace b9 {

struct FunctionEntry {
	const Instruction* instructions;
}

using FunctionMap = std::map<const char*, std::size_t>;
using FunctionTable = std::vector<const FunctionEntry>;

class Module {
public:
	/// Look up a function in this module.
	Instruction* function(const char* const name) const;

private:
	std::map<const char*, const Function
	std::vector<Instruction*
};

} // namespace b9

#endif // B9_MODULE_HPP_
