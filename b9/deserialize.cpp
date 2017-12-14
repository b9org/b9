#include <iostream>
#include <memory>
#include <vector>
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
 // TODO -> Update this if-statement as we add more section codes
 if (sectionCode != 1) {
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

bool printModule(std::istream &in, std::ostream &out) {
	static const char* modulePrint =
    "B9 Module:\n"
    "  Functions:\n";
  
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
    module->functions.emplace_back("unknown_function", std::vector<Instruction>{});
    FunctionDef& functionSpec = module->functions.back();;
    uint32_t index = module->getFunctionIndex(functionSpec.name);
    readFunctionData(in, functionSpec, index);
    
    if (!readInstructions(in, functionSpec.instructions)) {
      throw DeserializeException{"Error in read bytecodes"};  
    }
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
  default:
    throw DeserializeException{"Invalid Section Code"};
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
