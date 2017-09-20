#include <b9.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int fib(int n) {
  if (n < 3) return 1;
  return fib(n - 1) + fib(n - 2);
}

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

/* Main Loop */

bool run_test(ExecutionContext *context, const char *testName) {
  if (context->debug >= 2) {
    printf("Test \"%s\": starting\n", testName);
  }
  Instruction *func = getFunctionAddress(context, testName);
  if (func == nullptr) {
    printf("Test \"%s\": failed,  failed to load function\n", testName);
    return false;
  }
  const char *mode = hasJITAddress(func) ? "JIT" : "Interpreted";
  int result = interpret(context, func);
  if (!result) {
    printf("Mode %s, Test \"%s\": failed, returned %X\n", mode, testName,
           result);
  } else {
    if (context->debug >= 1) {
      printf("Mode %s, Test \"%s\": success, returned %X\n", mode, testName,
             result);
    }
  }
  return result;
}

extern "C" PrimitiveFunction test_primitive_return_5;
extern "C" void test_primitive_return_5(ExecutionContext *context) {
  push(context, 5);
}

extern "C" PrimitiveFunction test_primitive_take_2;
extern "C" void test_primitive_take_2(ExecutionContext *context) {
  int a = pop(context);
  int b = pop(context);
  push(context, 0);
}

extern "C" PrimitiveFunction test_primitive_take_2_add;
extern "C" void test_primitive_take_2_add(ExecutionContext *context) {
  int a = pop(context);
  int b = pop(context);
  push(context, a + b);
}

int main(int argc, char *argv[]) {
  ExecutionContext stackContext;
  ExecutionContext *context = &stackContext;

  b9_jit_init();

  parseArguments(context, argc, argv);

  bool result = true;

  if (!loadLibrary(context, "test.so")) {
    return 0;
  }

  test_validateFibResult(context);

  int count;
  for (count = 0; count < 2; count++) {
    // run all tests in the program which start with test_
    struct ExportedFunctionData *functions = context->functions;
    int functionIndex = 0;
    while (functions[functionIndex].name != NO_MORE_FUNCTIONS) {
      if (strncmp("test_", functions[functionIndex].name, 5) == 0) {
        run_test(context, functions[functionIndex].name);
      }
      functionIndex++;
    }
    generateAllCode(context);  // first time interpreted, second time JIT
  }

  if (result) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

#endif
