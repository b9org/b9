#include <iostream>
#include <iterator>
#include <string.h>

/* Parse uint32 */
uint32_t uint32Parser(std::istream &in, std::uint32_t &out) {
  in.read((char *)&out, sizeof(out));
  std::cout << "The uint32_t value is: " << out << std::endl;
  if (sizeof(out) != 4) {
    std::cout << "Incorrect size of uint32_t" << std::endl;
    return false;
  } else {
    return true;
  }
}

/* Read header "b9module" from module */
bool parseHeader(std::istream &in) {
  char buffer[8];
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
bool parseSectionCode(std::istream &in) {
 uint32_t sectionCode;
 if (!uint32Parser(in, sectionCode)) {
   return false;
 }
 if (sectionCode != 1) {
   std::cout << "Incorrect section code" << std::endl;
   return false;
 }
 std::cout << "Section Code: " << sectionCode << std::endl;
 // TODO Set section code in Module
 return true;
}

/* Read Function Count  */
bool parseFunctionCount(std::istream &in) {
  uint32_t functionCount;
  if (!uint32Parser(in, functionCount)) {
    return false; 
  }
  std::cout << "Function count: " << functionCount << std::endl;
  // TODO Set function count in module
  return true;
}

/* Read function information  */
bool parseFunctionData(std::istream &in) {
  uint32_t functionIndex;
  uint32_t nargs;
  uint32_t nregs;
  if (!uint32Parser(in, functionIndex)) {
    return false;
  }
  if (!uint32Parser(in, nargs)) {
    return false;
  }
  if (!uint32Parser(in, nregs)) {
    return false;
  }
  std::cout << "Function Index: " << functionIndex << std::endl;
  std::cout << "Number Arguments: " << nargs << std::endl;
  std::cout << "Number Registers: " << nregs << std::endl;
  // TODO Set function index, nargs, and nregs in Module
  return true;
}

/* Disassemble Binary Module  */
bool disassemble(std::istream &in, std::ostream &out) {
  // Read header
  if (parseHeader(in)) {
    std::cout << "Success in parseHeader" << std::endl;
  } else {
    std::cout << "Failure in parseHeader" << std::endl;
    return false;
  }
  // Read section code
  if (parseSectionCode(in)) {
    std::cout << "Success in parseSectionCode" << std::endl;
  } else {
    std::cout << "Failure in parseSectionCode" << std::endl;
    return false;
  }
  // Read function count 
  if (parseFunctionCount(in)) {
    std::cout << "Success in parseFunctionCount" << std::endl;
  } else {
    std::cout << "Failure in parseFunctionCount" << std::endl;
    return false;
  }
  // Read function data
  if (parseFunctionData(in)) {
    std::cout << "Success in parseFunctionData" << std::endl;
  } else {
    std::cout << "Failure in parseFunctionData" << std::endl;
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

