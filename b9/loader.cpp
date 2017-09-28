#include <b9/loader.hpp>
#include <b9/binaryformat.hpp>

namespace b9 {

std::shared_ptr<Module> Loader::module(const char* n) {

  std::shared_ptr<Module> module{};

  dlerror(); // clear error

  const std::string name{};
  name += "./";
  name += n;

  // open the module library

  void* handle = dlopen(name.c_str(), RTLD_NOW);
  char *error = dlerror();

  if (error != nullptr) {
    std::cerr "Failed to load" << n << ": "<< error << std::endl;
    return nullptr;
  }

  // Get the symbol table

  struct ExportedFunctionData *table =
      (struct ExportedFunctionData *)dlsym(handle, "b9_exported_functions");
  error = dlerror();
  if (error) {
    printf("%s\n", error);
    return false;
  }
  functions_ = table;

  // Get the primitive table

  PrimitiveData *primitives =
      (struct PrimitiveData *)dlsym(handle, "b9_primitives");
  error = dlerror();
  if (error) {
    printf("%s\n", error);
    return false;
  }
  primitives_ = primitives;

  // Get the string table

  const char **stringTable =
      (const char **)dlsym(handle, "b9_exported_strings");
  error = dlerror();
  if (error) {
    printf("%s\n", error);
    return false;
  }
  this->stringTable_ = stringTable;

  if (this->debug_ > 0) {
    for (int i = 0; i < functions_->functionCount_; i++) {
      FunctionSpecification *functionSpec = functions_->functionTable_ + i;
      std::cout << "Name: " << functionSpec->name_ << " byteCodes: " << functionSpec->byteCodes_;
    }
  }

  return true;
}

Instruction* Module::function(const char * const name) const {
	  Instruction* f = reinterpret_cast<Instruction *>(dlsym(library_, functionName));
	  char *error = dlerror();
	  if (error) {
		printf("%s\n", error);
		return nullptr;
	  }
	
	  return function;
}