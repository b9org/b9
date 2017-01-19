#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "b9.h"

int
main(int argc, char *argv[])
{
    ExecutionContext context;
    char sharelib[128];
    const char *mainFunction = "b9main";

    b9_jit_init();

    parseArguments(&context, argc, argv);

    /* Default to run bench.so */
    if (context.name == nullptr) {
        printf("No program was passed to b9, Running default benchmark for b9.\n");
        context.name = "./bench.so";
    }

    if (!loadLibrary(&context, context.name)) {
        return EXIT_FAILURE;
    }

    Instruction *function = getFunctionAddress(&context, mainFunction);
    if (function == nullptr) {
        return EXIT_FAILURE;
    }

    StackElement resultInterp = 0;
    StackElement resultJit = 0;
    long timeInterp = 0;
    long timeJIT = 0;

    printf("Running Interpreted, looping %d times\n", context.loopCount);
    printf("Options: DirectCall (%d), DirectParameterPassing (%d), UseVMOperandStack (%d)\n",
        context.directCall, context.passParameters, context.operandStack);

    resultInterp = timeFunction(&context, function, context.loopCount, &timeInterp);

    printf("Running JIT looping %d times\n", context.loopCount);
    generateAllCode(&context);

    resultJit = timeFunction(&context, function, context.loopCount, &timeJIT);

    printf("Result for Interp is %lld, resultJit is %lld\n", resultInterp, resultJit);
    printf("Time for Interp %ld ms, JIT %ld ms\n", timeInterp, timeJIT);
    printf("JIT speedup = %f\n", timeInterp * 1.0 / timeJIT);

    if (resultInterp == resultJit) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
