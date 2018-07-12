#include <b9/ExecutionContext.hpp>
#include <b9/Module.hpp>
#include <b9/VirtualMachine.hpp>
#include <b9/deserialize.hpp>
#include <b9/serialize.hpp>

#include <gtest/gtest.h>
#include <stdlib.h>
#include <strstream>
#include <vector>

namespace b9 {
namespace test {

/* Helper functions for creating test modules */

std::shared_ptr<Module> makeEmptyModule() {
  auto m = std::make_shared<Module>();
  return m;
}

std::shared_ptr<Module> makeSimpleModule() {
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i = {{OpCode::INT_PUSH_CONSTANT, 2},
                                {OpCode::INT_PUSH_CONSTANT, 2},
                                {OpCode::INT_ADD},
                                {OpCode::FUNCTION_RETURN},
                                END_SECTION};

  m->functions.push_back(b9::FunctionDef{"add_args", 0, i, 2, 4});
  m->strings = {"FruitBat", "cantaloupe", "123$#*"};

  return m;
}

std::shared_ptr<Module> makeComplexModule() {
  auto m = std::make_shared<Module>();
  std::vector<Instruction> f1 = {{OpCode::INT_PUSH_CONSTANT, 2},
                                 {OpCode::INT_PUSH_CONSTANT, 2},
                                 {OpCode::INT_ADD},
                                 {OpCode::FUNCTION_RETURN},
                                 END_SECTION};

  std::vector<Instruction> f2 = {{OpCode::PUSH_FROM_LOCAL, 0},
                                 {OpCode::PRIMITIVE_CALL, 0},
                                 {OpCode::DROP, 0},
                                 {OpCode::INT_PUSH_CONSTANT, 0},
                                 {OpCode::FUNCTION_RETURN, 0},
                                 END_SECTION};

  std::vector<Instruction> f3 = {{OpCode::PUSH_FROM_LOCAL, 0},
                                 {OpCode::PRIMITIVE_CALL, 1},
                                 {OpCode::DROP, 0},
                                 {OpCode::INT_PUSH_CONSTANT, 0},
                                 {OpCode::FUNCTION_RETURN, 0},
                                 END_SECTION};

  m->functions.push_back(b9::FunctionDef{"add_args", 0, f1, 1, 1});
  m->functions.push_back(b9::FunctionDef{"b9PrintString", 1, f2, 2, 2});
  m->functions.push_back(b9::FunctionDef{"b9PrintNumber", 2, f3, 3, 3});

  m->strings = {"mercury", "Venus", "EARTH", "mars", "JuPiTeR", "sAtUrN"};

  return m;
}

/* Unit Tests */

void roundTripStringSection(const std::vector<std::string>& strings) {
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  writeStringSection(buffer, strings);
  EXPECT_TRUE(buffer.good());

  std::vector<std::string> strings2;
  readStringSection(buffer, strings2);

  EXPECT_EQ(strings, strings2);
}

TEST(RoundTripSerializationTest, testStringSection) {
  roundTripStringSection({"mercury", "venus", "pluto"});
  roundTripStringSection({"", "", "", ""});
  roundTripStringSection({});
}

bool roundTripInstructions(std::vector<Instruction> instructions) {
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  writeInstructions(buffer, instructions);

  std::vector<Instruction> result;
  if (!readInstructions(buffer, result)) {
    return false;
  }

  EXPECT_EQ(instructions.size(), result.size());
  for (int i = 0; i < instructions.size(); i++) {
    EXPECT_EQ(instructions[i], result[i]);
  }
  return true;
}

TEST(RoundTripSerializationTest, testInstructions) {
  std::vector<Instruction> instructions1 = {{OpCode::INT_PUSH_CONSTANT, 2},
                                            {OpCode::INT_PUSH_CONSTANT, 2},
                                            {OpCode::INT_ADD},
                                            {OpCode::FUNCTION_RETURN},
                                            END_SECTION};

  std::vector<Instruction> instructions2 = {};

  std::vector<Instruction> instructions3 = {{OpCode::INT_PUSH_CONSTANT, 2},
                                            {OpCode::INT_PUSH_CONSTANT, 2},
                                            {OpCode::INT_ADD},
                                            {OpCode::FUNCTION_RETURN}};

  std::vector<Instruction> instructions4 = {END_SECTION};

  EXPECT_TRUE(roundTripInstructions(instructions1));
  EXPECT_FALSE(roundTripInstructions(instructions2));
  EXPECT_FALSE(roundTripInstructions(instructions3));
  EXPECT_TRUE(roundTripInstructions(instructions4));
}

void roundTripFunctionData(FunctionDef& f) {
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  writeFunctionData(buffer, f);
  EXPECT_TRUE(buffer.good());

  std::vector<Instruction> i2;
  auto f2 = FunctionDef("", 0, i2, 0, 0);
  readFunctionData(buffer, f2, 0);
  EXPECT_EQ(f, f2);
}

TEST(RoundTripSerializationTest, testFunctionData) {
  std::vector<Instruction> i1 = {{OpCode::INT_PUSH_CONSTANT, 2},
                                 {OpCode::INT_PUSH_CONSTANT, 2},
                                 {OpCode::INT_ADD},
                                 {OpCode::FUNCTION_RETURN},
                                 END_SECTION};
  auto f1 = FunctionDef("testName", 0, i1, 4, 5);

  std::vector<Instruction> i2 = {};
  auto f2 = FunctionDef("testName", 0, i2, 4, 5);

  roundTripFunctionData(f1);
  roundTripFunctionData(f2);
}

void roundTripFunctionSection(std::vector<FunctionDef> functions) {
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  writeFunctionSection(buffer, functions);
  EXPECT_TRUE(buffer.good());

  std::vector<FunctionDef> functions2;
  readFunctionSection(buffer, functions2);

  EXPECT_EQ(functions.size(), functions2.size());

  for (int i = 0; i < functions.size(); i++) {
    EXPECT_EQ(functions[i], functions2[i]);
  }
}

TEST(RoundTripSerializationTest, testFunctionSection) {
  std::vector<Instruction> i1 = {{OpCode::INT_PUSH_CONSTANT, 2},
                                 {OpCode::INT_PUSH_CONSTANT, 2},
                                 {OpCode::INT_ADD},
                                 {OpCode::FUNCTION_RETURN},
                                 END_SECTION};
  auto f1 = FunctionDef("testName", 0, i1, 4, 5);
  std::vector<FunctionDef> functions;
  functions.push_back(f1);

  roundTripFunctionSection(functions);
}

void roundTripSerializeDeserialize(std::shared_ptr<Module> module) {
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  serialize(buffer, *module);

  auto module2 = deserialize(buffer);

  EXPECT_EQ(*module, *module2);
  EXPECT_EQ(*module, *module);
  EXPECT_EQ(*module2, *module2);
}

TEST(RoundTripSerializationTest, testSerializeDeserialize) {
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);

  auto m1 = makeSimpleModule();
  roundTripSerializeDeserialize(m1);

  auto m2 = makeComplexModule();
  roundTripSerializeDeserialize(m2);

  auto m3 = std::make_shared<Module>();
  std::vector<Instruction> i = {{OpCode::INT_PUSH_CONSTANT, 2},
                                {OpCode::INT_PUSH_CONSTANT, 2},
                                {OpCode::INT_ADD},
                                {OpCode::FUNCTION_RETURN},
                                END_SECTION};
  auto f = FunctionDef("testName", 0, i, 4, 5);
  std::vector<FunctionDef> functions;
  functions.push_back(f);
  m3->functions = functions;
  roundTripSerializeDeserialize(m3);

  auto m4 = std::make_shared<Module>();
  std::vector<std::string> strings = {"sandwich", "RubberDuck", "Dumbledore"};
  m4->strings = strings;
  roundTripSerializeDeserialize(m4);
}

template <typename Number>
void roundTripNumber(std::vector<Number> numbers) {
  for (auto number : numbers) {
    std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
    EXPECT_TRUE(writeNumber(buffer, number));
    Number toRead;
    EXPECT_TRUE(readNumber(buffer, toRead));
    EXPECT_EQ(number, toRead);
  }
}

TEST(RoundTripSerializationTest, testWriteReadNumber) {
  std::vector<int> numbers1 = {-1, 0, 15, 250, 10000, -10000};
  std::vector<uint32_t> numbers2 = {20, 0, 375, 25000, 13000};
  std::vector<uint64_t> numbers3 = {500, 0, 825, 13000, 7200};
  std::vector<size_t> numbers4 = {250, 0, 37, 16000, 28000};

  roundTripNumber(numbers1);
  roundTripNumber(numbers2);
  roundTripNumber(numbers3);
  roundTripNumber(numbers4);
}

TEST(RoundTripSerializationTest, testWriteReadString) {
  for (auto string : {"power ranger", "", "the\0empty\0string"}) {
    std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
    writeString(buffer, string);
    std::string toRead;
    readString(buffer, toRead);
    EXPECT_EQ(string, toRead);
  }
}

TEST(ReadBinaryTest, testEmptyModule) {
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  EXPECT_THROW(deserialize(buffer), DeserializeException);
}

TEST(ReadBinaryTest, testCorruptModule) {
  std::stringstream buffer1(std::ios::in | std::ios::out | std::ios::binary);
  std::string corruptHeader = "b9mod";
  uint32_t sectionCode = 1;
  uint32_t functionCount = 4;
  writeString(buffer1, corruptHeader);
  writeNumber(buffer1, sectionCode);
  writeNumber(buffer1, functionCount);

  std::stringstream buffer2(std::ios::in | std::ios::out | std::ios::binary);
  sectionCode = 3;
  writeHeader(buffer2);
  writeNumber(buffer2, sectionCode);
  writeNumber(buffer2, functionCount);

  EXPECT_THROW(deserialize(buffer1), DeserializeException);
  EXPECT_THROW(deserialize(buffer2), DeserializeException);
}

TEST(ReadBinaryTest, runValidModule) {
  auto m1 = makeSimpleModule();
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  serialize(buffer, *m1);

  auto m2 = deserialize(buffer);
  Om::ProcessRuntime runtime;
  VirtualMachine vm(runtime, {});
  vm.load(m2);
  vm.run(0, {Om::Value(Om::AS_INT48, 1), Om::Value(Om::AS_INT48, 2)});
}

}  // namespace test
}  // namespace b9
