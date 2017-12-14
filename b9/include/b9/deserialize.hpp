#ifndef B9_DESERIALIZE_HPP_
#define B9_DESERIALIZE_HPP_

#include <b9/instructions.hpp>
#include <b9/module.hpp>

#include <iostream>
#include <memory>
#include <vector>
#include <string.h>

namespace b9 {

struct DeserializeException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline bool readBytes(std::istream& in, char* buffer, long bytes) {
  long count = 0;
  do {
    in.read(&buffer[count], bytes - count);
    count += in.gcount();
  } while (count < bytes && in.good());
  return count == bytes;
}

/* Generic number reader */
template <typename Number>
bool readNumber(std::istream &in, Number &out, long bytes = sizeof(Number)) {
  char* buffer = (char*)&out;
  return readBytes(in, buffer, bytes);
}

void readHeader(std::istream &in, char* buffer);

void readSectionCode(std::istream &in, uint32_t &sectionCode);

bool readFunctionCount(std::istream &in, uint32_t &functionCount);

void readFunctionData(std::istream &in, FunctionDef &functionSpec, uint32_t index);

bool readInstructions(std::istream &in, std::shared_ptr<std::vector<Instruction>> &instructions);

bool createModule(std::istream &in, std::ostream &out);

std::shared_ptr<Module> deserialize(std::istream &in);

} // b9 namespace

#endif // B9_DESERIALIZE_HPP_
