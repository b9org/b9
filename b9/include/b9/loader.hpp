#if !defined(B9_LOADER_HPP_)
#define B9_LOADER_HPP_

#include <b9/module.hpp>

#include <memory>
#include <stdexcept>
#include <string>

namespace b9 {

struct DlFunctionEntry {
  const char* name;
  const Instruction* address;
  std::uint32_t nargs;
  std::uint32_t nregs;
};

/// Raw dynamic library function table. Directly embedded in a DL.
/// The table is an array of FunctionSpec pointers which will be loaded into the
/// module.
struct DlFunctionTable {
  std::size_t length;
  const DlFunctionEntry* functions;
};

/// Raw dynamic library string table.
/// The string table is an array of char strings.
struct DlStringTable {
  std::size_t length;
  const char* const* strings;
};

/// Dynamic library loader exception.
struct DlException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/// A loader which constructs modules from native dynamic libraries.
class DlLoader {
 public:
  DlLoader(bool debug = false);

  /// Load a Module from a native dynamic library (DL).
  std::shared_ptr<Module> loadModule(const std::string& name) const;

 private:
  /// Load the DL into memory.
  void* openLibrary(const std::string& name) const;

  /// Load the raw function table from the DL into the module.
  void loadFunctions(const std::shared_ptr<Module>& module,
                     void* const handle) const;

  /// Load the raw dl string table from the DL into the module.
  void loadStrings(const std::shared_ptr<Module>& module, void* handle) const;

  /// Lookup the DL symbol, and return it's address.
  template <typename T = void>
  T* loadSymbol(void* handle, const char* symbol) const;

  bool debug_;
};

}  // namespace b9

#endif  // B9_LOADER_HPP_
