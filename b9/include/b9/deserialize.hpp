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
  return count == bytes;
}

/* Generic number reader */
template <typename Number>
bool readNumber(std::istream &in, Number &out, long bytes = sizeof(Number)) {
  char *buffer = (char *)&out;
  return readBytes(in, buffer, bytes);
}

void readHeader(std::istream &in, char *buffer);

void readFunctionData(std::istream &in, FunctionDef &functionSpec);

void readFunction(std::istream &in, FunctionDef &functionDef);

bool readInstructions(std::istream &in, std::vector<Instruction> &instructions);

void readString(std::istream &in, std::string &toRead);

void readFunctionSection(std::istream &in, std::vector<FunctionDef> &functions);

void readStringSection(std::istream &in, std::vector<std::string> &strings);

std::shared_ptr<Module> deserialize(std::istream &in);

}  // namespace b9

#endif  // B9_DESERIALIZE_HPP_
