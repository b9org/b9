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

template <typename Number>
bool readNumber(std::istream &in, Number &out, long bytes = sizeof(Number)) {
  char *buffer = (char *)&out;
  return readBytes(in, buffer, bytes);
}

void readString(std::istream &in, std::string &toRead) {
  uint32_t length;
  if (!readNumber(in, length, sizeof(length))) {
    throw DeserializeException{"Error reading string length"};
  }
  for (size_t i = 0; i < length; i++) {
    if (in.eof()) {
      throw DeserializeException{"Error reading string"};
    }
    char current = in.get();
    toRead.push_back(current);
  }
}

void readStringSection(std::istream &in, std::vector<std::string> &strings);

bool readInstructions(std::istream &in, std::vector<Instruction> &instructions);

void readFunctionData(std::istream &in, FunctionDef &functionSpec);

void readFunction(std::istream &in, FunctionDef &functionDef);

void readFunctionSection(std::istream &in, std::vector<FunctionDef> &functions);

void readSection(std::istream &in, std::shared_ptr<Module> &module);

void readHeader(std::istream &in, char *buffer);

std::shared_ptr<Module> deserialize(std::istream &in);

}  // namespace b9

#endif  // B9_DESERIALIZE_HPP_
