#ifndef B9_SERIALIZE_HPP_
#define B9_SERIALIZE_HPP_

#include <b9/module.hpp>
#include <fstream>
#include <iostream>

namespace b9 {

/* Generic number writer */
template <typename Number>
bool writeNumber(std::ofstream &out, const Number &n) {
  const long bytes = sizeof(Number);
  auto buffer = reinterpret_cast<const char *>(&n);
  out.write(buffer, bytes);
  return out.good();
}

inline void writeString(std::ofstream &out, std::string toWrite) {
  out << toWrite;
}

void writeInstructions(std::ofstream &out, const FunctionDef &functionDef);

void writeFunctionData(std::ofstream &out, const Module &module);

void writeFunctionSection(std::ofstream &out, const Module &module);

void writeStringSection(std::ofstream &out, const Module &module);

void serialize(const Module &module, std::ofstream &out);

}  // namespace b9

#endif  // B9_SERIALIZE_HPP_
