#ifndef B9_DESERIALIZE_HPP_
#define B9_DESERIALIZE_HPP_

#include <b9/instructions.hpp>
#include <b9/module.hpp>

#include <string.h>
#include <iostream>
#include <memory>
#include <vector>

namespace b9 {

struct DeserializeException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline bool readBytes(std::istream &in, char *buffer, long bytes) {
  long count = 0;
  do {
    in.read(&buffer[count], bytes - count);
    count += in.gcount();
  } while (count < bytes && in.good());
  if (count != bytes) {
    std::cout << "Error in readBytes" << std::endl;
    return false;
  }
  return true;
}

/* Generic number reader */
template <typename Number>
bool readNumber(std::istream &in, Number &out, long bytes = sizeof(Number)) {
  char *buffer = (char *)&out;
  if (!readBytes(in, buffer, bytes)) {
    std::cout << "Error in readNumber" << std::endl;
    return false;
  }
  return true;
}

void readHeader(std::istream &in, char *buffer);

void readSectionCode(std::istream &in, uint32_t &sectionCode);

bool readFunctionCount(std::istream &in, uint32_t &functionCount);

void readFunctionData(std::istream &in, FunctionDef &functionSpec,
                      uint32_t index);

bool readInstructions(std::istream &in,
                      std::shared_ptr<std::vector<Instruction>> &instructions);

void readString(std::istream &in, std::string &toRead, uint32_t length);

void readStringSection(std::istream &in, std::shared_ptr<Module> &module);

bool createModule(std::istream &in, std::ostream &out);

std::shared_ptr<Module> deserialize(std::istream &in);

void printModule(std::shared_ptr<Module> &module);

}  // namespace b9

#endif  // B9_DESERIALIZE_HPP_
