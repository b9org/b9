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
    generateCode(context->functions[0], context);
    for (i = 0; i <= 12; i++) {
        runFib(context, i);
    }
    removeAllGeneratedCode(context);
}

int
benchMarkFib(ExecutionContext *context)
{
    /* Load the fib program into the context, and validate that
     * the fib functions return the correct result */
    if (!loadLibrary(context, "./bench.so")) {
        return 0;
    }

    StackElement result = 0;

    /* make sure everything is not-jit'd for this initial bench
     * allows you to put examples above, tests etc, and not influence this
     * benchmark compare interpreted vs JIT'd */
    //  removeAllGeneratedCode(context);

    int LOOP = 200000;
    printf("Running %d loops, interpreted -> ", LOOP);

    long timeInterp = 0;
    long timeJIT = 0;
    {
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);

        result = interpret(context, context->functions[0]);

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);

        printf(" result is: %lld\n", result);
        timeInterp = (tval_result.tv_sec * 1000 + (tval_result.tv_usec / 1000));
    }

    /* Generate code for fib functions */
    // temp, only do fib for now, some issue in loops jit
    generateCode(context->functions[1], context);

    printf("Running %d loops, compiled -> ", LOOP);

    {
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);

        result = interpret(context, context->functions[0]);

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);

        printf(" result is: %lld\n", result);
        timeJIT = (tval_result.tv_sec * 1000 + (tval_result.tv_usec / 1000));
    }

    printf("Time for %d iterations Interp %ld ms JIT %ld ms\n", LOOP, timeInterp, timeJIT);
    printf("JIT speedup = %f\n", timeInterp * 1.0 / timeJIT);

    return 0;
}

bool
test_validateFibResult(ExecutionContext *context)
{

    if (!loadLibrary(context, "./bench.so")) {
        return 0;
    }
    validateFibResult(context);
    return true;
}

bool
test_benchMarkFib()
{
    ExecutionContext context;
    if (!loadLibrary(&context, "./bench.so")) {
        return 0;
    }
    benchMarkFib(&context);
    return true;
}

/* Main Loop */

int
main(int argc, char *argv[])
{
    ExecutionContext context;
    b9_jit_init();

    parseArguments(&context, argc, argv);

    bool result = true;
    result &= test_validateFibResult(&context);
    // result &= test_benchMarkFib();

    if (result) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
