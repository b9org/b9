#include <string.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <b9/instructions.hpp>
#include <b9/module.hpp>
#include <b9/serialize.hpp>

namespace b9 {

void writeStringSection(std::ostream &out,
                        const std::vector<std::string> &strings) {
  uint32_t stringCount = strings.size();
  if (!writeNumber(out, stringCount)) {
    throw SerializeException("Error writing string section");
  }
  for (auto string : strings) {
    writeString(out, string);
  }
}

bool writeInstructions(std::ostream &out,
                       const std::vector<Instruction> &instructions) {
  for (auto instruction : instructions) {
    if (!writeNumber(out, instruction)) {
      return false;
    }
  }
  return true;
}

void writeFunctionData(std::ostream &out, const FunctionDef &functionDef) {
  writeString(out, functionDef.name);
  bool ok = writeNumber(out, functionDef.index) &&
            writeNumber(out, functionDef.nargs) &&
            writeNumber(out, functionDef.nregs);
  if (!ok) {
    throw SerializeException("Error writing function data");
  }
}

void writeFunction(std::ostream &out, const FunctionDef &functionDef) {
  writeFunctionData(out, functionDef);
  if (!writeInstructions(out, functionDef.instructions)) {
    throw SerializeException("Error writing instructions");
  }
}

void writeFunctionSection(std::ostream &out,
                          const std::vector<FunctionDef> &functions) {
  uint32_t functionCount = functions.size();
  if (!writeNumber(out, functionCount)) {
    throw SerializeException("Error writing function count");
  }
  for (auto function : functions) {
    writeFunction(out, function);
  }
}

void writeSections(std::ostream &out, const Module &module) {
  if (module.functions.size() != 0) {
    uint32_t sectionCode = 1;
    if (!writeNumber(out, sectionCode)) {
      throw SerializeException("Error writing function section code");
    }
    writeFunctionSection(out, module.functions);
  }

  if (module.strings.size() != 0) {
    uint32_t sectionCode = 2;
    if (!writeNumber(out, sectionCode)) {
      throw SerializeException("Error writing string section code");
    }
    writeStringSection(out, module.strings);
  }
}

void writeHeader(std::ostream &out) {
  const char header[] = {'b', '9', 'm', 'o', 'd', 'u', 'l', 'e'};
  uint32_t length = 8;
  out.write(header, length);
  if (!out.good()) {
    throw SerializeException("Error writing header");
  }
}

void serialize(std::ostream &out, const Module &module) {
  writeHeader(out);
  writeSections(out, module);
}

}  // namespace b9
