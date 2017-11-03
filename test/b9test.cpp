#include <b9/interpreter.hpp>
#include <b9/loader.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>

#include <gtest/gtest.h>

namespace b9 {
namespace test {

// clang-format off
const std::vector<const char*> TEST_NAMES = {
  "test_return_true",
  "test_return_false",
  "test_add",
  "test_sub",
  // CASCON2017 add test_div and test_mul here
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
  "helper_test_string_return_string",
  "test_string_return_string",
  "test_while"
};
// clang-format on

class InterpreterTest : public ::testing::Test {
 public:
  std::shared_ptr<Module> module_;

  virtual void SetUp() {
    auto moduleName = getenv("B9_TEST_MODULE");
    ASSERT_NE(moduleName, nullptr);
    module_ = DlLoader{}.loadModule(moduleName);
  }
};

TEST_F(InterpreterTest, interpreter) {
  Config cfg;

  VirtualMachine vm{cfg};
  vm.load(module_);

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {})) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit) {
  Config cfg;
  cfg.jit = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {})) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_dc) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {})) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_pp) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {})) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_lvms) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;
  cfg.lazyVmState = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {})) << "Test Failed: " << test;
  }
}

TEST(MyTest, arguments) {
  b9::VirtualMachine vm{{}};
  auto m = std::make_shared<Module>();
  Instruction i[] = {{ByteCode::PUSH_FROM_VAR, 0},
                     {ByteCode::PUSH_FROM_VAR, 1},
                     {ByteCode::INT_ADD},
                     {ByteCode::FUNCTION_RETURN},
                     END_SECTION};

  m->functions.push_back(b9::FunctionSpec{"add_args", i, 2, 0});
  vm.load(m);
  auto r = vm.run("add_args", {1, 2});
  EXPECT_EQ(r, 3);
}

}  // namespace test
}  // namespace b9
