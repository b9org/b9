#include <dlfcn.h>
#include <b9/loader.hpp>

namespace b9 {

DlLoader::DlLoader(bool debug) : debug_{debug} {}

std::shared_ptr<Module> DlLoader::loadModule(const std::string& name) const {
  auto module = std::make_shared<Module>();
  auto handle = openLibrary(name);
  loadFunctions(module, handle);
  loadStrings(module, handle);
  loadPrimitives(module, handle);
  return module;
}

void* DlLoader::openLibrary(const std::string& name) const {
  auto n = name;  // TODO: prepend "./" for relative names.
  auto handle = dlopen(n.c_str(), RTLD_NOW);
  auto msg = dlerror();
  if (msg) throw DlException{msg};
  return handle;
};

void DlLoader::loadFunctions(const std::shared_ptr<Module>& module,
                             void* const handle) const {
  auto table = loadSymbol<const DlFunctionTable>(handle, "b9_function_table");
  for (std::size_t i = 0; i < table->length; i++) {
    auto& entry = table->functions[i];
    module->functions.emplace_back(entry.name, entry.address, entry.nargs, entry.nregs);
  }
}

void DlLoader::loadPrimitives(const std::shared_ptr<Module>& module,
                              void* handle) const {
  auto table = loadSymbol<const DlPrimitiveTable>(handle, "b9_primitive_table");
  for (std::size_t i = 0; i < table->length; i++) {
    auto primitive =
        loadSymbol<PrimitiveFunction>(RTLD_DEFAULT, table->primitives[i].name);
    module->primitives.push_back(primitive);
  }
}

void DlLoader::loadStrings(const std::shared_ptr<Module>& module,
                           void* handle) const {
  auto table = loadSymbol<const DlStringTable>(handle, "b9_string_table");
  for (std::size_t i = 0; i < table->length; i++) {
    module->strings.push_back(table->strings[i]);
  }
}

template <typename T>
T* DlLoader::loadSymbol(void* handle, const char* symbol) const {
  auto p = dlsym(handle, symbol);
  auto msg = dlerror();
  if (msg) throw DlException{msg};
  return reinterpret_cast<T*>(p);
};

#if 0
DlLoader::debugDump()
if (this->debug_ > 0) {
    for (int i = 0; i < functions_->functionCount_; i++) {
      FunctionSpecification *functionSpec = functions_->functionTable_ + i;
      std::cout << "Name: " << functionSpec->name_ << " byteCodes: " << functionSpec->byteCodes_;
    }
  }
#endif  // 0

}  // namespace b9
