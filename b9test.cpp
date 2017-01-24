#include "b9.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

int
fib(int n)
{
    if (n < 3)
        return 1;
    return fib(n - 1) + fib(n - 2);
}

void
runFib(ExecutionContext *context, int value)
{
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

void
validateFibResult(ExecutionContext *context)
{
    int i;
    for (i = 0; i <= 12; i++) {
        runFib(context, i);
    }
    generateCode(context, 0); // fib is program 0
    for (i = 0; i <= 12; i++) {
        runFib(context, i);
    }
    removeAllGeneratedCode(context);
}

bool
test_validateFibResult(ExecutionContext *context)
{
    // if (!loadLibrary(context, "./bench.so")) {
    //     return 0;
    // }
    validateFibResult(context);
    return true;
}

/* Main Loop */

bool
run_test(ExecutionContext *context, const char *testName)
{

    if (context->debug >= 1) {
        printf("Test \"%s\": starting\n", testName);
    }
    Instruction *func = getFunctionAddress(context, testName);
    if (func == nullptr) {
        printf ("Test \"%s\": failed,  failed to load function\n", testName);
        return false;
    }
    const char *mode = hasJITAddress(func) ? "JIT" : "Interpreted";
    int result = interpret(context, func);
    if (!result) {
        printf ("Mode %s, Test \"%s\": failed, returned %X\n", mode, testName, result);
    } else {
        if (context->debug >= 1) {
            printf("Mode %s, Test \"%s\": success, returned %X\n", mode, testName, result);
        }
    }
}

int
main(int argc, char *argv[])
{
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
    for (count =0; count < 2; count++) {  
        run_test(context, "test_add");
        run_test(context, "test_sub");
        run_test(context, "test_equal");
        run_test(context, "test_greaterThan");
        run_test(context, "test_greaterThanOrEqual");
        run_test(context, "test_lessThan");
        run_test(context, "test_lessThanOrEqual");
        run_test(context, "test_call");
        run_test(context, "test_while");

        generateAllCode(context); // first time interpreted, second time JIT
    }  

    if (result) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
