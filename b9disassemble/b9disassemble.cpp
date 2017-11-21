#include <iostream>
#include <string.h>

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

bool disassemble(std::istream &in, std::ostream &out) {
  // TODO call various parsing functions 
  return parseHeader(in);
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

