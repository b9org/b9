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

TEST (ReadBinaryTest, emptyModule) {
	std::ifstream in("empty.mod", std::ios::in | std::ios::binary);
	EXPECT_THROW(deserialize(in), DeserializeException);
}

TEST (ReadBinaryTest, validModule) {
  const char* pwd = getenv("PWD");
  std::cerr << pwd << std::endl;
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
	auto module = deserialize(in);
}

TEST (PrintModuleTest, validModule) {
  const char* pwd = getenv("PWD");
  std::cerr << pwd << std::endl;
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
	auto module = deserialize(in);
	printModule(module);
}

TEST (ReadBinaryTest, corruptModule) {
	std::ifstream in("corrupt.mod", std::ios::in | std::ios::binary);
  EXPECT_THROW(deserialize(in), DeserializeException);
}

/*TEST (ReadBinaryTest, readBinaryModule) {
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
  auto module = deserialize(in);
  for (auto it = module->functions.begin(); it != module->functions.end(); it++) {
    uint32_t index = module->getFunctionIndex(it->name);
    std::cout << "Function Data: " << std::endl 
      << "	index: " << index 
      << ", nargs: " << it->nargs 
      << ", nregs: " << it->nregs << std::endl;
    std::cout << "Instructions: " << std:: endl;
		for (auto instruction : it->instructions) {
      std::cout << std::hex;
      std::cout << "	Bytecode: " << instruction << std::endl;
      std::cout << std::dec; 
    }
  }
}*/

TEST (ReadBinaryTest, runValidModule) {
  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
	auto module = deserialize(in);
  VirtualMachine vm{{}}; 
  vm.load(module);
  vm.run(0,{1,2});
}

/*TEST (ReadBinaryTest, readStrings) {
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i =
    {{ByteCode::INT_PUSH_CONSTANT, 2},
    {ByteCode::INT_PUSH_CONSTANT, 2},
    {ByteCode::INT_ADD},
    {ByteCode::FUNCTION_RETURN},
    END_SECTION};
  m->functions.push_back(b9::FunctionDef{"add_args", i, 3, 4});

  std::ifstream in("valid.mod", std::ios::in | std::ios::binary);
  readStringSection(in, m);
}*/

TEST (WriteBinaryTest, writeModule) {
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i = {{ByteCode::INT_PUSH_CONSTANT, 2},
                     						{ByteCode::INT_PUSH_CONSTANT, 2},
                     						{ByteCode::INT_ADD},
                     						{ByteCode::FUNCTION_RETURN},
                     						END_SECTION};
  m->functions.push_back(b9::FunctionDef{"add_args", i, 3, 4});
  serialize(m);  
  const char* testString = "arianne";
  m->strings.push_back(testString);
}

}  // namespace test
}  // namespace b9

