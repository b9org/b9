#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <b9/loader.hpp>
#include <vector>
#include <strstream>

#include <gtest/gtest.h>
#include <b9/deserialize.hpp>
#include <b9/interpreter.hpp>
#include <b9/module.hpp>
#include <b9/serialize.hpp>

namespace b9 {
namespace test {

// SERIALIZE UNIT TESTS
// ********************
// writeInstructions(std::ostream &out, std::vector<Instruction> i) - DONE 
// writeFunctionData(std::ostream &out, const Module &module) - DONE 
// writeFunctionSection(std::ostream &out, const Module &module)
// writeStringSection(std::ostream &out, const Module &module)
// serialize(std::ostream &out, const Module &module)

// writeNumber(std::ostream &out, const Number &n)
// writeString(std::ostream &out, std::string toWrite)


TEST(SerializeTest, writeInstructions) {
  std::vector<Instruction> instructions = 
    {{ByteCode::INT_PUSH_CONSTANT, 2},
     {ByteCode::INT_PUSH_CONSTANT, 2},
     {ByteCode::INT_ADD},
     {ByteCode::FUNCTION_RETURN},
     END_SECTION};
  
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  writeInstructions(buffer, instructions);
	
  std::vector<Instruction> result;
	readInstructions(buffer, result);
  
  EXPECT_EQ(instructions.size(), result.size());
	for (int i = 0; i < instructions.size(); i++) {
    EXPECT_EQ(instructions[i], result[i]);
  }
}

TEST(SerializeTest, writeFunctionData) {
  std::string name = "testName";
  uint32_t index = 0;
  uint32_t nargs = 4;
  uint32_t nregs = 5;
  std::vector<Instruction> i1 = {{ByteCode::INT_PUSH_CONSTANT, 2},
                                 {ByteCode::INT_PUSH_CONSTANT, 2},
                                 {ByteCode::INT_ADD},
                                 {ByteCode::FUNCTION_RETURN},
                                 END_SECTION};
  auto f1 = FunctionDef(name, index, i1, nargs, nregs);
  
  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  writeFunctionData(buffer, f1);
  EXPECT_TRUE(buffer.good());
 
  std::vector<Instruction> i2;
  auto f2 = FunctionDef("", 0, i2, 0 ,0);
  readFunctionData(buffer, f2);
  EXPECT_EQ(f1, f2);
}

TEST(SerializeUnitTest, writeFunctionSection) {
  std::string name = "testName";
  uint32_t index = 0;
  uint32_t nargs = 4;
  uint32_t nregs = 5;
  std::vector<Instruction> i1 = {{ByteCode::INT_PUSH_CONSTANT, 2},
                                 {ByteCode::INT_PUSH_CONSTANT, 2},
                                 {ByteCode::INT_ADD},
                                 {ByteCode::FUNCTION_RETURN},
                                 END_SECTION};
  auto f1 = FunctionDef(name, index, i1, nargs, nregs);
  std::vector<FunctionDef> functions1;
  functions1.push_back(f1);

  std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
  writeFunctionSection(buffer, functions1);
  EXPECT_TRUE(buffer.good()); 

  std::vector<FunctionDef> functions2;
  std::vector<Instruction> instructions;
  functions2.push_back(b9::FunctionDef{"", 0, instructions, 0, 0});
  readFunctionSection(buffer, functions2);
  EXPECT_EQ(functions1.size(), functions2.size());

  for (int i = 0; i < functions1.size(); i++) {
    EXPECT_EQ(functions1[i], functions2[i]);
  }
}

/*
TEST(SerializeUnitTest, writeStringSection) {
}

TEST(SerializeUnitTest, serialize) {
}

TEST(SerializeUnitTest, writeNumber) {
}

TEST(SerializeUnitTest, writeString) {
}
*/



// DESERIALIZE UNIT TESTS
// **********************
// readHeader(std::istream &in)
// readInstructions(std::istream &in, std::vector<Instruction> &instructions)
// readFunctionData(std::istream &in, FunctionDef &functionDef)
// readFunction(std::istream &in, FunctionDef &functionDef)
// readFunctionSection(std::istream &in, std::vector<FunctionDef> &functions)
// readString(std::istream &in, std::string &toRead)
// readStringSection(std::istream &in, std::vector<std::string> &strings)
// readSection(std::istream &in, std::shared_ptr<Module> &module) 
// deserialize(std::istream &in)

// readBytes(std::istream &in, char *buffer, long bytes)
// readNumber(std::istream &in, Number &out, long bytes = sizeof(Number))



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

  serialize(out, *m);
}

TEST(ReadAndWriteBinary, readWriteModule) {
  std::ifstream in("complex.mod", std::ios::in | std::ios::binary);
  auto m = deserialize(in);
  std::ofstream out;
  out = std::ofstream("testReadWriteModule.mod", std::ios::binary);
  serialize(out, *m);
}

/*
TEST(WriteAndReadBinary, writeReadModule) {
	// Make a massive fucking module
  auto m = std::make_shared<Module>();
  std::vector<Instruction> f1 = {{ByteCode::INT_PUSH_CONSTANT, 2},
                                {ByteCode::INT_PUSH_CONSTANT, 2},
                                {ByteCode::INT_ADD},
                                {ByteCode::FUNCTION_RETURN},
                                END_SECTION};

	std::vector<Instruction> f2 = {{ByteCode::PUSH_FROM_VAR, 0},
																 {ByteCode::PRIMITIVE_CALL, 0},
																 {ByteCode::DROP, 0},
																 {ByteCode::INT_PUSH_CONSTANT, 0},
																 {ByteCode::FUNCTION_RETURN, 0},
																 END_SECTION};

	std::vector<Instruction> f3 = {{ByteCode::PUSH_FROM_VAR, 0},
																 {ByteCode::PRIMITIVE_CALL, 1},
																 {ByteCode::DROP, 0},
																 {ByteCode::INT_PUSH_CONSTANT, 0},
																 {ByteCode::FUNCTION_RETURN, 0},
																 END_SECTION};
	m->functions.push_back(b9::FunctionDef{"add_args", index, f1, 1, 1});
	m->functions.push_back(b9::FunctionDef{"b9PrintString", index, f2, 2, 2});
	m->functions.push_back(b9::FunctionDef{"b9PrintNumber", index, f3, 3, 3});

  std::string testString1 = "mercury";
  std::string testString2 = "Venus";
  std::string testString3 = "EARTH";
  std::string testString4 = "mars";
  std::string testString5 = "JuPiTeR";
  std::string testString6 = "sAtUrN";
  m->strings.push_back(testString1);
  m->strings.push_back(testString2);
  m->strings.push_back(testString3);
  m->strings.push_back(testString4);
  m->strings.push_back(testString5);
  m->strings.push_back(testString6);

	// Serialize dat shit
  std::ofstream out;
  out = std::ofstream("testWriteReadModule.mod", std::ios::binary);
  serialize(out, *m);
	
	// Deserialize dat shit back 	
  std::ifstream in("testWriteReadModule.mod", std::ios::in | std::ios::binary);
  auto m2 = deserialize(in);

	// Compare dem modules 
			
	


  // Create a module (1) 
  // Serialize to binary
  // Deserialize back to module (2)
  // Compare module (1) with module (2) 
}*/

}  // namespace test
}  // namespace b9
