#include <iostream>
#include <memory>
#include <vector>
#include <string.h>

#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>

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
void createModule(std::istream &in, std::ostream &out) {
  

}


} // b9 namespace
