#ifndef B9_ASSEMBLE_HPP_
#define B9_ASSEMBLE_HPP_

#include <b9/instructions.hpp>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace b9 {

void assembleStringTable(std::istream &in, std::ostream &out);

void assembleInstruction(std::istream &in, std::ostream &out);

void assembleFunctionData(std::istream &in, std::ostream &out);

void assembleFunction(std::istream &in, std::ostream &out);

void assemble(std::istream &in, std::ostream &out);

} // namespace b9

#endif  // B9_ASSEMBLE_HPP_

