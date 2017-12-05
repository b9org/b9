#include <iostream>
#include <fstream> 
#include <memory>
#include <vector>
#include <string.h>

#include <b9/module.hpp>
#include <b9/instructions.hpp>

namespace b9 {

/* Write bytecode array to file  */
bool writeBytecodes(std::ofstream &out, FunctionSpec &functionSpec) {
	for (auto instruction : functionSpec.instructions) {
    out << instruction;
	}
	return true;
} 

bool parseModule (const std::shared_ptr<Module> &module) {
	// Make new file "testModule.mod"
	std::ofstream testModule;
	testModule.open("testModule.mod");
	// Define first 3 fields
	std::string header = "b9module";
	uint32_t sectionCode = 1;
	uint32_t functionCount = sizeof(module->functions);
  // Output first 3 fields to file
	testModule << header << sectionCode << functionCount; 
	// Output function data for each function to file 
  for (int i = 0; i < functionCount; i++) {
		std::string name = module->functions[i].name;
		uint32_t functionIndex = module->findFunction(name);
		testModule << functionIndex << module->functions[i].nargs << module->functions[i].nregs;
	  writeBytecodes(testModule, module->functions[i]);
	}
	return true;
}

} // namespace b9
