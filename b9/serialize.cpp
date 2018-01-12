#include <string.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <b9/instructions.hpp>
#include <b9/module.hpp>
#include <b9/serialize.hpp>

namespace b9 {

void writeInstructions(std::ostream &out, const std::vector<Instruction> &instructions) {
  for (auto instruction : instructions) {
    if (!writeNumber(out, instruction)) {
      throw SerializeException("Error writing instructions");
    }
  }
}

void writeFunctionData(std::ostream &out, const FunctionDef &functionDef) {
  writeString(out, functionDef.name);
  writeNumber(out, functionDef.index);
  writeNumber(out, functionDef.nargs);
  writeNumber(out, functionDef.nregs);
  writeInstructions(out, functionDef.instructions);
}

void writeFunctionSection(std::ostream &out, const std::vector<FunctionDef> &functions) {
  uint32_t sectionCode = 1;
  uint32_t functionCount = functions.size();
  writeNumber(out, sectionCode);
  writeNumber(out, functionCount);
  for (auto function : functions) {
    writeFunctionData(out, function);
  }
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
  writeFunctionSection(out, module.functions);
  writeStringSection(out, module);
}

}  // namespace b9
