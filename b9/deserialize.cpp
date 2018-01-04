#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <string.h>

#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>
#include <b9/module.hpp>

namespace b9 {

void readHeader(std::istream &in) {
  if (in.peek() == std::istream::traits_type::eof()) {
 		throw DeserializeException{"Empty Input File"};
  }

  const char magic[] = { 'b', '9', 'm', 'o', 'd', 'u', 'l', 'e'};
	const long bytes = sizeof(magic);

	char buffer[bytes];
	bool ok = readBytes(in, buffer, bytes);
  if (!ok || strncmp(magic, buffer, bytes) != 0) {
   	throw DeserializeException{"Corrupt Header"}; 
  }
}

void readSectionCode(std::istream &in, uint32_t &sectionCode) {
 if (!readNumber(in, sectionCode)) {
   throw DeserializeException{"Error in Parse Section Code"};
 }
 // TODO -> Update as we add more section codes
 if (sectionCode != 1 || sectionCode != 2) {
   throw DeserializeException{"Error in Parse Section Code"};
 }
}

bool readInstructions(std::istream &in, std::vector<Instruction> &instructions) {
  do {
    RawInstruction instruction;
    if (!readNumber(in, instruction)) {
      return false;
    }
    instructions.emplace_back(instruction);
  } while (instructions.back() != END_SECTION);
  return true;
}

void readFunctionData(std::istream& in, FunctionDef &functionSpec, uint32_t index) {
  readNumber(in, index);
  readNumber(in, functionSpec.nargs);
  readNumber(in, functionSpec.nregs);
}

void readFunctionSection(std::istream& in, std::shared_ptr<Module>& module) {
  uint32_t functionCount;
  if (!readNumber(in, functionCount)) {
		throw DeserializeException{"Error reading function count"};
	}
  for (uint32_t i = 0; i < functionCount; i++) {
    std::string functionName;
		uint32_t size;
		readNumber(in, size);
    readString(in, functionName, size);
    module->functions.emplace_back(functionName, std::vector<Instruction>{});
    FunctionDef& functionSpec = module->functions.back();
		uint32_t index = module->getFunctionIndex(functionSpec.name);
    readFunctionData(in, functionSpec, index);
    
    if (!readInstructions(in, functionSpec.instructions)) {
      throw DeserializeException{"Error in read instructions"};
    }
   }
}

void readString(std::istream &in, std::string &toRead, uint32_t length) {
  for (size_t i = 0; i < length; i++) {
    if (in.eof()) {
      throw DeserializeException{"Unexpected EOF"};
    }
    char current = in.get();
    toRead.push_back(current);
  }
}

void readStringSection(std::istream &in, std::shared_ptr<Module> &module) {
  uint32_t stringCount;
  if (!readNumber(in, stringCount, sizeof(stringCount))) {
    throw DeserializeException{"Error reading string count"};
  }
  for (uint32_t i = 0; i < stringCount; i++) {
    uint32_t length;
    std::string toRead;
    if (!readNumber(in, length, sizeof(length))) {
      throw DeserializeException{"Error reading string length"};
    }
    readString(in, toRead, length);
    module->strings.push_back(toRead);
  }
}

void readSection(std::istream& in, std::shared_ptr<Module>& module) { 
	uint32_t sectionCode;
	bool ok = readNumber(in, sectionCode);

  if (!ok) {
    throw DeserializeException{"Failed to read section code"};
  }

	switch(sectionCode) {
  case 1:
	  return readFunctionSection(in, module);
  case 2:
    return readStringSection(in, module);
  default:
    throw DeserializeException{"Invalid Section Code"};
  }
}

void printModule(std::shared_ptr<Module>& module) {
	uint32_t functionCount = sizeof(module->functions);
  uint32_t index = 0;
  for (auto it = module->functions.begin(); it != module->functions.end(); it++) {
    std::cout << "Function Data at index " << index << ": " << std::endl
      << "   Name: " << it->name
      << ", Number Arguments: " << it->nargs
      << ", Number Registers: " << it->nregs << std::endl;
    std::cout << "   Instructions: " << std:: endl;
    for (auto instruction : it->instructions) {
      std::cout << std::hex;
      std::cout << "      " << instruction << std::endl;
      std::cout << std::dec;
    }
		++index;
  }
  std::cout << "String Table:" << std::endl;
  for (auto string : module->strings) {
    std::cout << "   " << string << std::endl;
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

} // b9 namespace
