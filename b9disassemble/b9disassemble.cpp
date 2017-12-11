#include <iostream>
#include <memory>
#include <vector>
#include <string.h>

#include <b9/module.hpp>
#include <b9/serialize.hpp>
#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>

using namespace b9;

extern "C" int main (int argc, char** argv) {
  // Test disassemble binary module 
  bool ok = disassemble(std::cin);
  if (!ok) {
    std::cout << "Failure in disassemble" << std::endl;
  } else {
    std::cout << "Success in disassemble" << std::endl;
  }    
  // Test serialize module
	auto m = std::make_shared<Module>();
  Instruction i[] = {{ByteCode::INT_PUSH_CONSTANT, 1},
                     {ByteCode::INT_PUSH_CONSTANT, 2},
                     {ByteCode::INT_ADD, 0},
                     {ByteCode::FUNCTION_RETURN, 0},
                     END_SECTION};
  m->functions.push_back(b9::FunctionSpec{"simple_add", i, 2, 2});
  ok = parseModule(m);
	if (!ok) {
		std::cout << "Failure in serialize" << std::endl;
	} else {
		std::cout << "Success in serialize" << std::endl;
	}
	return 0;
}

