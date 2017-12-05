#ifndef B9_DESERIALIZE_HPP_
#define B9_DESERIALIZE_HPP_

#include <iostream>
#include <memory>
#include <vector>
#include <string.h>

#include <b9/instructions.hpp>

namespace b9 {

/* Generic number parser */
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
bool parseHeader(std::istream &in, char* buffer);

/* Read Section Code  */
bool parseSectionCode(std::istream &in, uint32_t &sectionCode);

/* Read Function Count  */
bool parseFunctionCount(std::istream &in, uint32_t &functionCount);

/* Read function data  */
bool parseFunctionData(std::istream &in, std::vector<uint32_t> &functionData);

/* Read the bytecodes */
bool readInstructions(std::istream &in, std::shared_ptr<std::vector<Instruction>> &instructions);

/* Create Module  */
void createModule(std::istream &in, std::ostream &out);

} // b9 namespace
#endif // B9_DESERIALIZE_HPP_
