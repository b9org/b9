#include <iostream>
#include <iterator>
#include <string.h>

// Parse uint32
uint32_t uint32Parser(std::istream &in) {
  uint32_t value;
  in.read((char *)&value, sizeof(value));
  std::cout << "The value is: " << value << std::endl;
  
  return 0;
}


// Read header "b9module" from module 
bool parseHeader(std::istream &in) {
  char buffer[8];
  in.read(buffer, 8);
  auto gcount = in.gcount();
  if (gcount != 8)
    return false;
  if (0 != strncmp("b9module", buffer, gcount))
    return false;
  return true;
}

int parseSectionCode(std::istream &in) {
 return 0;
}


bool disassemble(std::istream &in, std::ostream &out) {
 parseHeader(in);
 uint32Parser(in);
 //parseSectionCode(in);
 // TODO call various parsing functions 
 return true;
}

int main (int argc, char** argv) {
  bool ok = disassemble(std::cin, std::cout);
  if (!ok) {
    std::cout << "Fail" << std::endl;
    return 1;
  }
  else {
    std::cout << "Success" << std::endl;
    return 0;
  }
}

