#include <iostream>
#include <memory>
#include <vector>
#include <string.h>

#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>
#include <b9/module.hpp>

namespace b9 {

/* Read header "b9module" from module */
bool parseHeader(std::istream &in, char* buffer) {
  in.read(buffer, 8);
  auto gcount = in.gcount();
  if (gcount != 8) {
    std::cout << "Error: Corrupt header" << std::endl;
    return false;
  }
  if (0 != strncmp("b9module", buffer, gcount)) {
    std::cout << "Error: Corrupt header" << std::endl;
    return false;
  }
  // TODO Set header in Module
  return true;
}

/* Read Section Code  */
bool parseSectionCode(std::istream &in, uint32_t &sectionCode) {
 if (!parseNumber(in, sectionCode, sizeof(sectionCode))) {
   return false;
 }
 // TODO update this if-statement as we add more section codes
 if (sectionCode != 1) {
   std::cout << "Incorrect section code" << std::endl;
   return false;
 }
 // TODO Set section code in Module
 return true;
}

/* Read Function Count  */
bool parseFunctionCount(std::istream &in, uint32_t &functionCount) {
  if (!parseNumber(in, functionCount, sizeof(functionCount))) {
    return false; 
  }
  // TODO Set function count in module
  return true;
}

/* Read function data  */
// Arianne: make a struct of 3 integers to hold these bois (look in module.hpp) 
bool parseFunctionData(std::istream &in, std::vector<uint32_t> &functionData) {
  for (size_t i = 0; i < 3; i++) {
    if (!parseNumber(in, functionData[i])) 
      return false;
  }
  // TODO Set function index, nargs, and nregs in Module
  return true;
}

/* Read the bytecodes */
bool readInstructions(std::istream &in, std::shared_ptr<std::vector<Instruction>> &instructions) {
  do {
    RawInstruction instruction;
    if (!parseNumber(in, instruction)) {
      return false;
    }
    instructions->emplace_back(instruction);
  } while (instructions->back() != END_SECTION);
  // TODO Set bytecodes in Module 
  return true;
}

/* Create Module  */
bool createModule(std::istream &in, std::ostream &out) {
  auto m = std::make_shared<Module>();
  //m->functions.push_back(b9::FunctionSpec{functionName, instructions, nargs, nregs});
	return true;	
}

/* Print Module */
bool printModule(std::istream &in, std::ostream &out) {
	// Print the Module
	static const char* modulePrint =
    "B9 Module:\n"
    "  Functions:\n";
  
  return true;    
}

bool disassemble(std::istream &in) {
  // Check for empty file
  if (in.peek() == std::istream::traits_type::eof()) {
    std::cout << "Error: Empty input file" << std::endl;
    return false;
  }
  // Read header
  char buffer[8];
  if (parseHeader(in, buffer)) {
    std::cout << "Success in parseHeader: " << buffer <<  std::endl;
  } else {
    std::cout << "Failure in parseHeader" << std::endl;
    return false;
  }
  // Read section code
  uint32_t sectionCode;
  if (parseSectionCode(in, sectionCode)) {
    std::cout << "Success in parseSectionCode: " << sectionCode << std::endl;
  } else {
    std::cout << "Failure in parseSectionCode" << std::endl;
    return false;
  }
  // Read function count
  uint32_t functionCount;
  if (parseFunctionCount(in, functionCount)) {
    std::cout << "Success in parseFunctionCount: " << functionCount << std::endl;
  } else {
    std::cout << "Failure in parseFunctionCount" << std::endl;
		return false;
	}
	// Read function data
  for (int i = 0; i < functionCount; i++) {
    std::vector<uint32_t> functionData(3);
    if (parseFunctionData(in, functionData)) {
      std::cout << "Success in parseFunctionData" << std::endl;
      std::cout << "Function index: " << functionData[0] << std::endl;
      std::cout << "Number Arguments: " << functionData[1] << std::endl;
      std::cout << "Number Registers: " << functionData[2] << std::endl;
    } else {
      std::cout << "Failure in parseFunctionData" << std::endl;
    }
    // Read bytecodes
    auto instructions = std::make_shared<std::vector<Instruction>>();
    if (readInstructions(in, instructions)) {
      std::cout << "Success in readBytecodes" << std::endl;
      std::cout << std::hex;
      for (auto instruction : *instructions) {
        std::cout << "Bytecode: " << instruction << std::endl;
      }
      std::cout << std::dec;
    } else {
      std::cout << "Failure in readBytecodes" << std::endl;
      return false;
    }
  }

  return true;
}

} // b9 namespace
