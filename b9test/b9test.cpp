#include <b9.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <gtest/gtest.h>

#if 0
int fib(int n) {
  if (n < 3) return 1;
  return fib(n - 1) + fib(n - 2);
}

namespace b9 {
namespace test {

class InterpreterTest : public ::testing::Test {
protected:
  VirtualMachine virtualMachine_;
  virtual void SetUp() {

    virtualMachine_.initialize();
    ASSERT_TRUE(virtualMachine_.loadLibrary("libinterpreter_test.so"));
  }

  virtual void TearDown() {
    ASSERT_TRUE(virtualMachine_.shutdown());
  }
};

TEST_F(InterpreterTest, test_return_true) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_return_true");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_return_false) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_return_false");
  ASSERT_FALSE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_add){
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_add");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_sub) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_sub");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_equal) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_equal");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_equal_1) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_equal_1");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_greaterThan) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_greaterThan");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_greaterThan_1) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_greaterThan_1");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_greaterThanOrEqual) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_greaterThanOrEqual");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_greaterThanOrEqual_1) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_greaterThanOrEqual_1");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_greaterThanOrEqual_2) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_greaterThanOrEqual_2");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_lessThan) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_lessThan");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_lessThan_1) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_lessThan_1");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_lessThan_2) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_lessThan_2");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_lessThan_3) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_lessThan_3");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_lessThanOrEqual) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_lessThanOrEqual");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_lessThanOrEqual_1) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_lessThanOrEqual_1");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_call) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_call");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_string_declare_string_var) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_string_declare_string_var");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, helper_test_string_return_string) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("helper_test_string_return_string");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_string_return_string) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_string_return_string");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

TEST_F(InterpreterTest, test_while) {
  b9::Instruction *function = virtualMachine_.getFunctionAddress("test_while");
  ASSERT_TRUE(virtualMachine_.runFunction(function));
}

} // namespace test
} // namespace b9

#if 0

void runFib(ExecutionContext *context, int value) {
  Instruction *func = getFunctionAddress(context, "fib_function");
  if (func == nullptr) {
    printf("Fail: failed to load fib function\n");
    return;
  }

  const char *mode = hasJITAddress(func) ? "JIT" : "Interpreted";

  int validate = fib(value);

  push(context, value);
  StackElement result = interpret(context, func);

  if (result == validate) {
    if (context->debug >= 1) {
      printf("Success: Mode <%s> fib %d returned %lld\n", mode, value, result);
    }
  } else {
    printf("Fail: Mode <%s> fib %d returned %lld\n", mode, value, result);
  }
}

void validateFibResult(ExecutionContext *context) {
  int i;
  for (i = 0; i <= 12; i++) {
    runFib(context, i);
  }
  // search for fib_function by name and force it to be compiled
  int functionIndex = 0;
  while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
    const char *fibfunc = "fib_function";
    if (strncmp(fibfunc, context->functions[functionIndex].name,
                strlen(fibfunc)) == 0) {
      generateCode(context, functionIndex);
    }
    functionIndex++;
  }
  for (i = 0; i <= 12; i++) {
    runFib(context, i);
  }
  removeAllGeneratedCode(context);
}

bool test_validateFibResult(ExecutionContext *context) {
  // if (!loadLibrary(context, "./bench.so")) {
  //     return 0;
  // }
  validateFibResult(context);
  return true;
}

#endif

#endif // 0