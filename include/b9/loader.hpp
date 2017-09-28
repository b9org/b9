#if !defined(B9_LOADER_HPP_)
#define B9_LOADER_HPP_

#include <b9/core.hpp>
#include <b9/module.hpp>
#include <b9/function.hpp>
#include <shared_ptr>

namespace b9 {

class Loader {
public:
	/// Load a Module.
	std::shared_ptr<Module> module(const char* name);
};

} // namespace b9

#endif // B9_LOADER_HPP_
