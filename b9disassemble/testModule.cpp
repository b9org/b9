#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <b9/loader.hpp>
#include <vector>

#include <gtest/gtest.h>
#include <b9/deserialize.hpp>
#include <b9/interpreter.hpp>
#include <b9/module.hpp>
#include <b9/serialize.hpp>

namespace b9 {
namespace test {

TEST(ReadBinaryTest, emptyModule) {
  std::ifstream in("empty.mod", std::ios::in | std::ios::binary);
  EXPECT_THROW(deserialize(in), DeserializeException);
}

TEST(ReadBinaryTest, simpleModule) {
  const char* pwd = getenv("PWD");
  std::cerr << pwd << std::endl;
  std::ifstream in("simple.mod", std::ios::in | std::ios::binary);
  auto module = deserialize(in);
}

TEST(PrintModuleTest, validModules) {
  const char* pwd = getenv("PWD");
  std::cerr << pwd << std::endl;

  std::cout << "*****TEST PRINT SIMPLE BINARY MODULE*****" << std::endl;
  std::ifstream in1("simple.mod", std::ios::in | std::ios::binary);
  auto module1 = deserialize(in1);

  std::cout << *module1;

  std::cout << "*****TEST PRINT COMPLEX BINARY MODULE*****" << std::endl;
  std::ifstream in2("complex.mod", std::ios::in | std::ios::binary);
  auto module2 = deserialize(in2);
  std::cout << *module2;
}

TEST(ReadBinaryTest, corruptModule) {
  std::ifstream in("corrupt.mod", std::ios::in | std::ios::binary);
  EXPECT_THROW(deserialize(in), DeserializeException);
}

TEST(ReadBinaryTest, runValidModule) {
  std::ifstream in("simple.mod", std::ios::in | std::ios::binary);
  auto module = deserialize(in);
  VirtualMachine vm{{}};
  vm.load(module);
  vm.run(0, {1, 2});
}

TEST(WriteBinaryTest, writeModule) {
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i = {{ByteCode::INT_PUSH_CONSTANT, 2},
                                {ByteCode::INT_PUSH_CONSTANT, 2},
                                {ByteCode::INT_ADD},
                                {ByteCode::FUNCTION_RETURN},
                                END_SECTION};
  uint32_t index = 0;
  m->functions.push_back(b9::FunctionDef{"add_args", index, i, 3, 4});
  std::string testString1 = "FruitBat";
  std::string testString2 = "cantelope";
  std::string testString3 = "123$#*";
  m->strings.push_back(testString1);
  m->strings.push_back(testString2);
  m->strings.push_back(testString3);

  std::ofstream out;
  out = std::ofstream("testWriteModule.mod", std::ios::binary);

  serialize(*m, out);
}

TEST(ReadAndWriteBinary, readWriteModule) {
  std::ifstream in("complex.mod", std::ios::in | std::ios::binary);
  auto module = deserialize(in);
  std::ofstream out;
  out = std::ofstream("testReadWriteModule.mod", std::ios::binary);
  serialize(*module, out);
}

}  // namespace test
}  // namespace b9
