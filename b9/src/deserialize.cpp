#include <string.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>
#include <b9/Module.hpp>

namespace b9 {

void readStringSection(std::istream &in, std::vector<std::string> &strings) {
  uint32_t stringCount;
  if (!readNumber(in, stringCount)) {
    throw DeserializeException{"Error reading string count"};
  }
  for (uint32_t i = 0; i < stringCount; i++) {
    std::string toRead;
    readString(in, toRead);
    strings.push_back(toRead);
  }
}

bool readInstructions(std::istream &in,
                      std::vector<Instruction> &instructions) {
  do {
    RawInstruction instruction;
    if (!readNumber(in, instruction)) {
      return false;
    }
    instructions.emplace_back(instruction);
  } while (instructions.back() != END_SECTION);
  return true;
}

void readFunctionData(std::istream &in, FunctionDef &functionDef) {
  readString(in, functionDef.name);
  bool ok = readNumber(in, functionDef.index) &&
            readNumber(in, functionDef.nparams) &&
            readNumber(in, functionDef.nlocals);
  if (!ok) {
    throw DeserializeException{"Error reading function data"};
  }
}

void readFunction(std::istream &in, FunctionDef &functionDef) {
  readFunctionData(in, functionDef);
  if (!readInstructions(in, functionDef.instructions)) {
    throw DeserializeException{"Error reading instructions"};
  }
}

void readFunctionSection(std::istream &in,
                         std::vector<FunctionDef> &functions) {
  uint32_t functionCount;
  if (!readNumber(in, functionCount)) {
    throw DeserializeException{"Error reading function count"};
  }
  for (uint32_t i = 0; i < functionCount; i++) {
    functions.emplace_back("", -1, std::vector<Instruction>{});
    readFunction(in, functions.back());
    if (functions[i].index != i) {
      throw DeserializeException{"Invalid index"};
    }
  }
}

void readSection(std::istream &in, std::shared_ptr<Module> &module) {
  uint32_t sectionCode;
  if (!readNumber(in, sectionCode)) {
    throw DeserializeException{"Error reading section code"};
  }

  switch (sectionCode) {
    case 1:
      return readFunctionSection(in, module->functions);
    case 2:
      return readStringSection(in, module->strings);
    default:
      throw DeserializeException{"Invalid Section Code"};
  }
}

void readHeader(std::istream &in) {
  if (in.peek() == std::istream::traits_type::eof()) {
    throw DeserializeException{"Empty Input File"};
  }

  const char magic[] = {'b', '9', 'm', 'o', 'd', 'u', 'l', 'e'};
  const std::size_t bytes = sizeof(magic);

  char buffer[bytes];
  bool ok = readBytes(in, buffer, bytes);
  if (!ok || strncmp(magic, buffer, bytes) != 0) {
    throw DeserializeException{"Corrupt Header"};
  }
}

std::shared_ptr<Module> deserialize(std::istream &in) {
  auto module = std::make_shared<Module>();
  readHeader(in);
  while (in.peek() != std::istream::traits_type::eof()) {
    readSection(in, module);
  }
  return module;
}

}  // namespace b9
