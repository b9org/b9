#include <string.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <b9/instructions.hpp>
#include <b9/module.hpp>
#include <b9/serialize.hpp>

namespace b9 {

void writeInstructions(std::ostream &out, const FunctionDef &functionDef) {
  std::cout << std::hex;
  for (auto instruction : functionDef.instructions) {
    writeNumber(out, instruction);
  }
  std::cout << std::dec;
}

void writeFunctionData(std::ostream &out, const Module &module) {
  for (auto function : module.functions) {
    writeString(out, function.name);
    writeNumber(out, function.index);
    writeNumber(out, function.nargs);
    writeNumber(out, function.nregs);
    writeInstructions(out, function);
  }
}

void writeFunctionSection(std::ostream &out, const Module &module) {
  uint32_t sectionCode = 1;
  uint32_t functionCount = module.functions.size();
  writeNumber(out, sectionCode);
  writeNumber(out, functionCount);
  writeFunctionData(out, module);
}

void writeStringSection(std::ostream &out, const Module &module) {
  uint32_t sectionCode = 2;
  uint32_t stringCount = module.strings.size();
  writeNumber(out, sectionCode);
  writeNumber(out, stringCount);
  for (auto string : module.strings) {
    writeString(out, string);
  }
}

void serialize(std::ostream &out, const Module &module) {
  auto f = module.functions;
  const char header[] = {'b', '9', 'm', 'o', 'd', 'u', 'l', 'e'};
  uint32_t length = 8;
  out.write(header, length);
  writeFunctionSection(out, module);
  writeStringSection(out, module);
}

}  // namespace b9
