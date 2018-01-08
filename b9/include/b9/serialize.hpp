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

void writeString(std::ofstream &out, std::string toWrite) { out << toWrite; }

void writeBytecodes(std::ofstream &, FunctionDef &);
void serialize(const std::shared_ptr<Module> &, std::string fileName);

}  // namespace b9

#endif  // B9_SERIALIZE_HPP_
