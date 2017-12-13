#include <b9/loader.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>

#include <gtest/gtest.h>
#include <b9/serialize.hpp>
#include <b9/deserialize.hpp>
#include <b9/interpreter.hpp>
#include <b9/module.hpp>

namespace b9 {
namespace test {

TEST(ParseBinaryTest, emptyModule) {
	std::ifstream in("empty.mod", std::ios::in | std::ios::binary);
	EXPECT_THROW(deserialize(in), DeserializeException);
}

TEST (ParseBinaryTest, validModule) {
  const char* pwd = getenv("PWD");
  std::cerr << pwd << std::endl;
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
	auto module = deserialize(in);
}

TEST(ParseBinaryTest, corruptModule) {
	std::ifstream in("corrupt.mod", std::ios::in | std::ios::binary);
  EXPECT_THROW(deserialize(in), DeserializeException);
}

TEST (ReadModuleTest, readBinaryModule) {
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
  auto module = deserialize(in);
  for (auto it = module->functions.begin(); it != module->functions.end(); it++) {
    std::cout << "Function Data: " << std::endl 
      << "	index: " << it->index 
      << ", nargs: " << it->nargs 
      << ", nregs: " << it->nregs << std::endl;
    std::cout << "Instructions: " << std:: endl;
		for (auto instruction : it->instructions) {
      std::cout << std::hex;
      std::cout << "	Bytecode: " << instruction << std::endl;
      std::cout << std::dec;  
		}
  }
}

TEST(ParseBinaryTest, runValidModule) {
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
	auto module = deserialize(in);
  VirtualMachine vm{{}}; 
  vm.load(module);
  vm.run(0,{1,2});
}


}  // namespace test
}  // namespace b9

