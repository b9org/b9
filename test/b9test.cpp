#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <b9/ExecutionContext.hpp>
#include <b9/deserialize.hpp>
#include <fstream>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>

namespace b9 {
namespace test {

namespace Om = OMR::Om;
namespace GC = OMR::GC;

// clang-format off
const std::vector<const char*> TEST_NAMES = {
  "test_return_true",
  "test_return_false",
  "test_add",
  "test_sub",
  "test_mul",
  "test_div",
  "test_equal",
  "test_equal_1",
  "test_greaterThan",
  "test_greaterThan_1",
  "test_greaterThanOrEqual",
  "test_greaterThanOrEqual_1",
  "test_greaterThanOrEqual_2",
  "test_lessThan",
  "test_lessThan_1",
  "test_lessThan_2",
  "test_lessThan_3",
  "test_lessThanOrEqual",
  "test_lessThanOrEqual_1",
  "test_call",
  "test_string_declare_string_var",
  "test_string_return_string",
  "test_while",
  "test_for_never_run_body",
  "test_for_sum"
};
// clang-format on

OMR::Runtime runtime;

class InterpreterTest : public ::testing::Test {
 public:
  std::shared_ptr<Module> module_;

  virtual void SetUp() {
    auto moduleName = getenv("B9_TEST_MODULE");
    ASSERT_NE(moduleName, nullptr);

    std::ifstream file(moduleName, std::ios_base::in | std::ios_base::binary);

    module_ = b9::deserialize(file);
  }
};

TEST_F(InterpreterTest, interpreter) {
  Config cfg;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInt48()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit) {
  Config cfg;
  cfg.jit = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInt48()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_dc) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInt48()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_pp) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInt48()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_lvms) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;
  cfg.lazyVmState = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInt48()) << "Test Failed: " << test;
  }
}

TEST(MyTest, arguments) {
  Config cfg;
  cfg.jit = true;
  b9::VirtualMachine vm{runtime, cfg};
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i = {{OpCode::PUSH_FROM_PARAM, 0},
                                {OpCode::PUSH_FROM_PARAM, 1},
                                {OpCode::INT_ADD},
                                {OpCode::FUNCTION_RETURN},
                                END_SECTION};
  m->functions.push_back(b9::FunctionDef{"add_args", i, 2, 0});
  vm.load(m);
  auto r = vm.run("add_args", {{Om::AS_INT48, 1}, {Om::AS_INT48, 2}});
  EXPECT_EQ(r, Om::Value(Om::AS_INT48, 3));
}

TEST(MyTest, jitSimpleProgram) {
  Config cfg;
  cfg.jit = true;
  b9::VirtualMachine vm{runtime, cfg};
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i = {{OpCode::INT_PUSH_CONSTANT, 0xdead},
                                {OpCode::FUNCTION_RETURN},
                                END_SECTION};
  m->functions.push_back(b9::FunctionDef{"add", i, 0, 0});
  vm.load(m);
  vm.generateAllCode();
  auto r = vm.run("add", {});
  EXPECT_EQ(r, Om::Value(Om::AS_INT48, 0xdead));
}

TEST(MyTest, haveAVariable) {
  Config cfg;
  cfg.jit = true;
  b9::VirtualMachine vm{runtime, cfg};
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i = {{OpCode::INT_PUSH_CONSTANT, 0xdead},
                                {OpCode::FUNCTION_RETURN},
                                END_SECTION};
  m->functions.push_back(b9::FunctionDef{"add", i, 0, 0});
  vm.load(m);
  vm.generateAllCode();
  auto r = vm.run("add", {});
  EXPECT_EQ(r, Om::Value(Om::AS_INT48, 0xdead));
}

TEST(ObjectTest, allocateSomething) {
  b9::VirtualMachine vm{runtime, {}};
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i = {
      {OpCode::NEW_OBJECT},            // new object
      {OpCode::POP_INTO_LOCAL, 0},     // store object into var0
      {OpCode::STR_PUSH_CONSTANT, 0},  // push "Hello, World"
      {OpCode::PUSH_FROM_LOCAL, 0},    // push var0 aka object
      {OpCode::POP_INTO_OBJECT, 0},  // pop "Hello, World" into object at slot 0
      {OpCode::SYSTEM_COLLECT},      // GC. Object is kept alive by var0
      {OpCode::PUSH_FROM_LOCAL, 0},  // push object
      {OpCode::PUSH_FROM_OBJECT, 0},  // get the string back
      {OpCode::PRIMITIVE_CALL, 0},    // call b9_prim_print_string
      {OpCode::FUNCTION_RETURN},      // finish with constant 0
      END_SECTION};
  m->strings.push_back("Hello, World");
  m->functions.push_back(b9::FunctionDef{"allocate_object", i, 0, 1});
  vm.load(m);
  Om::Value r = vm.run("allocate_object", {});
  EXPECT_EQ(r, Om::Value(Om::AS_INT48, 0));
}

}  // namespace test
}  // namespace b9
