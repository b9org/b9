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
    uint32_t nameSize = (function.name).length();
    writeNumber(out, nameSize);
    writeString(out, function.name);
    writeNumber(out, function.index);
    writeNumber(out, function.nargs);
    writeNumber(out, function.nregs);
    writeInstructions(out, function);
  }
}

void writeFunctionSection(std::ofstream &out, const std::shared_ptr<Module> &module) {
 	uint32_t sectionCode = 1;
	uint32_t functionCount = module->functions.size();
	writeNumber(out, sectionCode);
	writeNumber(out, functionCount);
  writeFunctionData(out, module);
}

void writeStringSection(std::ofstream &out, const std::shared_ptr<Module> module) {
  uint32_t sectionCode = 2;
  uint32_t stringCount = module->strings.size();
  writeNumber(out, sectionCode);
  writeNumber(out, stringCount);
  for (auto string : module->strings) {
    uint32_t stringLength = string.length();
    writeNumber(out, stringLength);
    writeString(out, string);
  }
}

void serialize (const std::shared_ptr<Module> &module, std::string fileName) {
  std::ofstream out;
  auto f = module->functions;
  out = std::ofstream(fileName, std::ios::binary);
  std::string header = "b9module";
  writeString(out, header);
	writeFunctionSection(out, module);
  writeStringSection(out, module);
}

} // namespace b9
