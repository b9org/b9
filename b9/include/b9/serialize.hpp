#ifndef B9_SERIALIZE_HPP_
#define B9_SERIALIZE_HPP_

#include <b9/module.hpp>
#include <fstream>
#include <iostream>

namespace b9 {

/* Generic number writer */
template <typename Number>
bool writeNumber(std::ostream &out, const Number &n) {
  const long bytes = sizeof(Number);
  auto buffer = reinterpret_cast<const char *>(&n);
  out.write(buffer, bytes);
  return out.good();
}

inline void writeString(std::ostream &out, std::string toWrite) {
  uint32_t length = toWrite.length();
  writeNumber(out, length);
  out << toWrite;
}

void writeInstructions(std::ostream &out, const FunctionDef &functionDef);

void writeFunctionData(std::ostream &out, const Module &module);

void writeFunctionSection(std::ostream &out, const Module &module);

void writeStringSection(std::ostream &out, const Module &module);

void serialize(std::ostream &out, const Module &module);

}  // namespace b9

#endif  // B9_SERIALIZE_HPP_
