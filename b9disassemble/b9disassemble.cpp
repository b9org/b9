#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <string.h>

#include <b9/instructions.hpp>

using namespace b9;

template <typename Number>
bool parseNumber(std::istream &in, Number &out, long bytes = sizeof(Number)) {
  long count = 0;
  char* buffer = (char*)&out;
  do {
    in.read(&buffer[count], bytes);
    count += in.gcount();
  } while (count < bytes && in.good());
  std::cout << "The value is: " << out << std::endl;
  if (count != bytes) {
    return false;
  }
  return true;
}

/* Read header "b9module" from module */
bool parseHeader(std::istream &in, char* buffer) {
  in.read(buffer, 8);
  auto gcount = in.gcount();
  if (gcount != 8)
    return false;
  if (0 != strncmp("b9module", buffer, gcount))
    return false;
  // TODO Set header in Module
  return true;
}

/* Read Section Code  */
bool parseSectionCode(std::istream &in, uint32_t &sectionCode) {
 if (!parseNumber(in, sectionCode, sizeof(sectionCode))) {
   return false;
 }
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
bool parseFunctionData(std::istream &in, std::vector<uint32_t> &functionData) {
  if (!parseNumber(in, functionData[0], sizeof(functionData[0]))) {
    return false;
  }
  if (!parseNumber(in, functionData[1], sizeof(functionData[1]))) {
    return false;
  }
  if (!parseNumber(in, functionData[2], sizeof(functionData[2]))) {
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
void createModule(std::istream &in, std::ostream &out) {
  

}

/* Disassemble Binary Module  */
bool disassemble(std::istream &in, std::ostream &out) {
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
  return true;
}

int main (int argc, char** argv) {
  bool ok = disassemble(std::cin, std::cout);
  if (!ok) {
    std::cout << "Failure in disassemble" << std::endl;
    return 1;
  } else {
    std::cout << "Success in disassemble" << std::endl;
    return 0;
  }
}
