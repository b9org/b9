#include <b9.hpp>
#include <b9/loader.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <gtest/gtest.h>

namespace b9 {
namespace test {

class InterpreterTestEnvironment : public ::testing::Environment {
  public:
    static const char* moduleName;

    virtual void SetUp() {
      moduleName = getenv("B9_TEST_MODULE");
    }
};

const char* InterpreterTestEnvironment::moduleName{nullptr};

class InterpreterTest : public ::testing::TestWithParam<const char*> {
public:
  static std::shared_ptr<Module> module_;
  VirtualMachine virtualMachine_{{}};

  static void SetUpTestCase() {
    DlLoader loader{};
    module_ = loader.loadModule(InterpreterTestEnvironment::moduleName);
  }

  virtual void SetUp() {
    virtualMachine_.load(module_);
  }
};

std::shared_ptr<Module> InterpreterTest::module_{nullptr};

TEST_P(InterpreterTest, run) {
  EXPECT_TRUE(virtualMachine_.run(GetParam()));
}

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
    "test_while"
));

} // namespace test
} // namespace b9

extern "C" int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  AddGlobalTestEnvironment(new b9::test::InterpreterTestEnvironment{});
  return RUN_ALL_TESTS();
}

