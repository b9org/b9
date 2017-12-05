#ifndef B9_SERIALIZE_HPP_
#define B9_SERIALIZE_HPP_

#include <iostream>
#include <fstream>
#include <b9/module.hpp>

namespace b9 {

bool writeBytecodes(std::ofstream&, FunctionSpec&);
bool parseModule (const std::shared_ptr<Module>&);

} // namspace b9

#endif  // B9_SERIALIZE_HPP_

