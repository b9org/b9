#include <iostream>
#include <fstream> 
#include <memory>
#include <vector>
#include <string.h>

#include <b9/serialize.hpp>
#include <b9/module.hpp>
#include <b9/instructions.hpp>

namespace b9 {

/* Write bytecode array to file  */
bool writeBytecodes(std::ofstream &out, FunctionDef &functionSpec) {
	for (auto instruction : functionSpec.instructions) {
    out << instruction;
	}
  std::cout << std::dec;
	//writeNumber(out, END_SECTION);
  return true;
} 

bool parseModule (const std::shared_ptr<Module> &module) {
	// Make new file "testModule.mod"
	std::ofstream testModule;
  testModule = std::ofstream("testModule.mod",/* std::ios::out |*/ std::ios::binary);
	//testModule.open("testModule.mod");
	// Define first 3 fields
	const char* const header = "b9module";
	std::cout << std::hex;
  uint32_t sectionCode = 1;
	uint32_t functionCount = module->functions.size();
  std::cout << "Section code is: " << sectionCode << std::endl;
  std::cout << "Function count is: " << functionCount << std::endl;
  std::cout << std::dec;
  // Output first 3 fields to file
	testModule << header;
  writeNumber(testModule, sectionCode);
  writeNumber(testModule, functionCount);
	// Output function data for each function to file 
  for (int i = 0; i < functionCount; i++) {
		std::string name = module->functions[i].name;
		uint32_t functionIndex = module->getFunctionIndex(name);
		writeNumber(testModule, functionIndex);
    writeNumber(testModule, module->functions[i].nargs);
    writeNumber(testModule, module->functions[i].nregs);
	  writeBytecodes(testModule, module->functions[i]);
	}
	return true;
}

} // namespace b9
