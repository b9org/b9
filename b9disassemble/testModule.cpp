#include <b9/interpreter.hpp>
#include <b9/loader.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>

#include <gtest/gtest.h>
#include <b9/serialize.hpp>
#include <b9/deserialize.hpp>

namespace b9 {
namespace test {

TEST (ParseBinaryTest, parseBinaryModule) {
  const char* pwd = getenv("PWD");
  std::cerr << pwd << std::endl;
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
	bool ok = disassemble(in);
  if (!ok) {
    std::cout << "Failure in disassemble using valid input file" << std::endl;
  } else {
    std::cout << "Success in disassemble using valid input file" << std::endl;
  }
	std::ifstream in2("empty.mod", std::ios::in | std::ios::binary);
	ok = disassemble(in2);
	if (!ok) {
		std::cout << "Test failed as expected using empty input file" << std::endl;
	} else {
		std::cout << "Test passed but was expected to fail using empty input file" << std::endl;
	}
	std::ifstream in3("corrupt.mod", std::ios::in | std::ios::binary);
	ok = disassemble(in3);
	if (!ok) {
		std::cout << "Test failed as expected using corrupt input file" << std::endl;
	} else {
		std::cout << "Test passed but was expected to fail using corrupt input file" << std::endl;
	}	
}

TEST (ReadModuleTest, readModule) {
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i =
    {{ByteCode::INT_PUSH_CONSTANT, 1},
                     {ByteCode::INT_PUSH_CONSTANT, 2},
                     {ByteCode::INT_ADD, 0},
                     {ByteCode::FUNCTION_RETURN, 0},
                     END_SECTION};
  m->functions.push_back(b9::FunctionSpec{"simple_add", i, 2, 2});
  parseModule(m);
}
  
}  // namespace test
}  // namespace b9

