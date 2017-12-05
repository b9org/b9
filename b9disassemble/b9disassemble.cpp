#include <iostream>
#include <memory>
#include <vector>
#include <string.h>

#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>

using namespace b9;

/* Disassemble Binary Module  */
bool disassemble(std::istream &in, std::ostream &out) {
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
