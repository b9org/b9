#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "b9.h"

int
main(int argc, char *argv[])
{
    b9_jit_init();
    ExecutionContext context;
    char sharelib[128];

    char *program = "b9main";

    /* Command Line Arguments */
    for (int i = 1; i < argc; i++) {
        char *name = argv[i];

        if (!strcmp(name, "-help")) {
            printf("-loop run the program a certain number of times");
            continue;
        }

        if (!strcmp(name, "-loop")) {
            context.loopCount = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-verbose")) {
            context.verbose = 1;
            continue;
        }

        if (!strcmp(name, "-debug")) {
            context.debug++;
            printf("debug is %d \n", context.debug);
            continue;
        }

        if (!strcmp(name, "-directcall")) {
            context.directCall = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-passparameters")) {
            context.passParameters = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-operandstack")) {
            context.operandStack = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-program")) {
            program = argv[i + 1];
            i++;
            continue;
        }

        context.name = name;
        continue;
    }

    /* Default to run bench.so */
    if (context.name == nullptr) {
        printf("No program was passed to b9, Running default benchmark for b9, looping 200000.\n");
        printf("Options: DirectCall (%d), DirectParameterPassing (%d), UseVMOperandStack (%d)\n",
            context.directCall, context.passParameters, context.operandStack);
        context.name = "./bench.so";
        context.loopCount = 1;
    }

    loadProgram(&context, context.name);

    StackElement resultInterp = 0;
    StackElement resultJit = 0;
    long timeInterp = 0;
    long timeJIT = 0;

    printf("Running Interpreted, looping %d times\n", context.loopCount);
    printf("Options: DirectCall (%d), DirectParameterPassing (%d), UseVMOperandStack (%d)\n",
        context.directCall, context.passParameters, context.operandStack);

    resultInterp = timeProgram(&context, 0, context.loopCount, &timeInterp);

    printf("Running JIT looping %d times\n", context.loopCount);
    generateAllCode(&context);

    resultJit = timeProgram(&context, 0, context.loopCount, &timeJIT);

    printf("Result for Interp is %lld, resultJit is %lld\n", resultInterp, resultJit);
    printf("Time for Interp %ld ms, JIT %ld ms\n", timeInterp, timeJIT);
    printf("JIT speedup = %f\n", timeInterp * 1.0 / timeJIT);

    if (resultInterp == resultJit) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
