#include <iostream>
#include <fstream> 
#include <memory>
#include <vector>
#include <string.h>

#include <b9/serialize.hpp>
#include <b9/module.hpp>
#include <b9/instructions.hpp>

namespace b9 {

void writeInstructions(std::ofstream &out, FunctionDef &functionSpec) {
	std::cout << std::hex;
  for (auto instruction : functionSpec.instructions) {
    writeNumber(out, instruction);
	}
  std::cout << std::dec;
}

void writeFunctionData(std::ofstream &out, const std::shared_ptr<Module> &module) {
	for (auto function : module->functions) {
    uint32_t index = module->getFunctionIndex(function.name);
    writeNumber(out, index);
    writeNumber(out, function.nargs);
    writeNumber(out, function.nregs);
	  writeInstructions(out, function);
	}
}

void serialize (const std::shared_ptr<Module> &module) {
  std::ofstream out;
  auto f = module->functions;
  // TODO implement some form of module naming 
  out = std::ofstream("test.mod", std::ios::binary);
  const char* const header = "b9module";
	out << header;
 	uint32_t sectionCode = 1;
	uint32_t functionCount = module->functions.size();
	writeNumber(out, sectionCode);
	writeNumber(out, functionCount);
	
	if (module->functions.size() > 0) {
		writeFunctionData(out, module);
	}
}

} // namespace b9
