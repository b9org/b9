#ifndef B9_SERIALIZE_HPP_
#define B9_SERIALIZE_HPP_

#include <b9/module.hpp>
#include <fstream>
#include <iostream>

namespace b9 {

struct SerializeException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

template <typename Number>
bool writeNumber(std::ostream &out, const Number &n) {
  const long bytes = sizeof(Number);
  auto buffer = reinterpret_cast<const char *>(&n);
  out.write(buffer, bytes);
  return out.good();
}

inline void writeString(std::ostream &out, std::string toWrite) {
  uint32_t length = toWrite.length();
  if (!writeNumber(out, length)) {
    throw SerializeException{"Error writing string length"};
  }
  out << toWrite;
  if (!out.good()) {
    throw SerializeException{"Error writing string"};
  }
}

void writeStringSection(std::ostream &out,
                        const std::vector<std::string> &strings);

bool writeInstructions(std::ostream &out,
                       const std::vector<Instruction> &instructions);

void writeFunctionData(std::ostream &out, const FunctionDef &functionDef);

void writeFunction(std::ostream &out, const FunctionDef &functionDef);

void writeFunctionSection(std::ostream &out,
                          const std::vector<FunctionDef> &functions);

void writeSections(std::ostream &out, const Module &module);

void writeHeader(std::ostream &out);

void serialize(std::ostream &out, const Module &module);

}  // namespace b9

#endif  // B9_SERIALIZE_HPP_
