#include <b9/interpreter.hpp>
#include <b9/loader.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>

#include <gtest/gtest.h>

namespace b9 {
namespace test {

class InterpreterTestEnvironment : public ::testing::Environment {
 public:
  static const char* moduleName;

  virtual void SetUp() {
    moduleName = getenv("B9_TEST_MODULE");
    ASSERT_NE(moduleName, nullptr);
  }
};

const char* InterpreterTestEnvironment::moduleName{nullptr};

class InterpreterTest : public ::testing::TestWithParam<const char*> {
 public:
  static std::shared_ptr<Module> module_;

  static void SetUpTestCase() {
    module_ = DlLoader{}.loadModule(InterpreterTestEnvironment::moduleName);
  }

  virtual void SetUp() {}
};

std::shared_ptr<Module> InterpreterTest::module_{nullptr};

TEST_P(InterpreterTest, run) {
  Config cfg;

  VirtualMachine vm{cfg};
  vm.load(module_);
  EXPECT_TRUE(vm.run(GetParam(), {}));
}

TEST_P(InterpreterTest, runJit) {
  Config cfg;
  cfg.jit = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  EXPECT_TRUE(vm.run(GetParam(), {}));
}

TEST_P(InterpreterTest, runDirectCall) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  EXPECT_TRUE(vm.run(GetParam(), {}));
}

TEST_P(InterpreterTest, runPassParam) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  EXPECT_TRUE(vm.run(GetParam(), {}));
}

TEST_P(InterpreterTest, runLazyVmState) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;
  cfg.lazyVmState = true;

  VirtualMachine vm{cfg};
  vm.load(module_);
  EXPECT_TRUE(vm.run(GetParam(), {}));
}

// clang-format off

INSTANTIATE_TEST_CASE_P(InterpreterTestSuite, InterpreterTest,
  ::testing::Values(
    "test_return_true",
    "test_return_false",
    "test_add",
    "test_sub",
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
    "test_while",
    "test_fib"
));

// clang-format on

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

extern "C" int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  AddGlobalTestEnvironment(new b9::test::InterpreterTestEnvironment{});
  return RUN_ALL_TESTS();
}
