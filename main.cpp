#include "b9.h"

#include <dlfcn.h>
#include <sys/time.h>


/* Byte Code Implementations */

void
push(ExecutionContext *context, StackElement value)
{
    *context->stackPointer = value;
    ++(context->stackPointer);
}

StackElement
pop(ExecutionContext *context)
{
    return *(--context->stackPointer);
}

void
bc_call(ExecutionContext *context, Parameter value)
{
    Instruction *program = context->functions[value];
    // printf("inside call\n");
    StackElement result = interpret(context, program);
    // printf("return from  call %d\n", result);

    push(context, result);
}

void
bc_push_from_arg(ExecutionContext *context, StackElement *args, Parameter offset)
{
    //  printf("bc_push_from_arg[%d] %d\n", offset, args[offset]);
    push(context, args[offset]);
}

void
bc_pop_into_arg(ExecutionContext *context, StackElement *args, Parameter offset)
{
    args[offset] = pop(context);
    //   printf("bc_pop_into_arg[%d] %d\n", offset, args[offset]);
}

void
bc_drop(ExecutionContext *context)
{
    pop(context);
}

void
bc_push_constant(ExecutionContext *context, Parameter value)
{
    //   printf("bc_push_constant %d\n", value);
    push(context, value);
}

Parameter
bc_jmple(ExecutionContext *context, Parameter delta)
{

    // push(left); push(right); if (left <= right) jmp
    StackElement right = pop(context);
    StackElement left = pop(context);
    // printf("jmple left %d, right %d\n", left, right);
    if (left <= right) {
        return delta;
    }
    return 0;
}

void
bc_add(ExecutionContext *context)
{
    // a+b is push(a);push(b); add
    StackElement right = pop(context);
    StackElement left = pop(context);
    // printf("add right %d left %d\n", right, left);
    StackElement result = left + right;

    push(context, result);
}

void
bc_sub(ExecutionContext *context)
{
    // left-right is push(left);push(right); sub
    StackElement right = pop(context);
    StackElement left = pop(context);
    StackElement result = left - right;

    push(context, result);
}

/* ByteCode Interpreter */

StackElement
interpret(ExecutionContext *context, Instruction *program)
{
    uint64_t *address = (uint64_t *)(&program[1]);
    if (*address) {
        StackElement result = 0;

#if PASS_PARAMETERS_DIRECTLY
        int argsCount = progArgCount(*program); 
        //printf("about to call jit args %d\n", argsCount);
        switch (argsCount) {
            case 0:{
                Interpret jitedcode = (Interpret)*address;
                result = (*jitedcode)(context, program);
            } break;
           case  1: {
                Interpret_1_args jitedcode = (Interpret_1_args)*address;
                StackElement p1 = pop(context);
                result = (*jitedcode)(context, program, p1);
            }
            break;
            case 2:{
                Interpret_2_args jitedcode = (Interpret_2_args)*address;
                StackElement p2 = pop(context);
                StackElement p1 = pop(context);
                result = (*jitedcode)(context, program, p1, p2);
            } break;
            case 3:{
                Interpret_3_args jitedcode = (Interpret_3_args)*address;
                StackElement p3 = pop(context);
                StackElement p2 = pop(context);
                StackElement p1 = pop(context);
                result = (*jitedcode)(context, program, p1, p2, p3);
            }break;
            default: 
                printf ("Need to add handlers for more parameters\n");
                break;
        } 
#else
        Interpret jitedcode = (Interpret)*address;
        result = (*jitedcode)(context, program);
#endif 
        return result;
    } 

    int nargs = progArgCount(*program);
    int tmps = progTmpCount(*program);
    //printf("Prog Arg Count %d, tmp count %d\n", nargs, tmps);

    Instruction *instructionPointer = program + 3;
    StackElement *args = context->stackPointer - nargs;
    context->stackPointer += tmps; // local storage for temps

    while (*instructionPointer != NO_MORE_BYTECODES) {
        // b9PrintStack(context);
        // printf("about to run %d %d\n", getByteCodeFromInstruction(*instructionPointer), getParameterFromInstruction(*instructionPointer));
        switch (getByteCodeFromInstruction(*instructionPointer)) {
        case PUSH_CONSTANT:
            // printf("push\n");
            bc_push_constant(context, getParameterFromInstruction(*instructionPointer));
            break;
        case DROP:
            //  printf("drop\n");
            bc_drop(context);
            break;
        case ADD:
            //  printf("add\n");
            bc_add(context);
            break;
        case SUB:
            //  printf("sub\n");
            bc_sub(context);
            break;
        case JMPLE:
            //  printf("jmple\n");
            instructionPointer += bc_jmple(context, getParameterFromInstruction(*instructionPointer));
            break;
        case CALL:
            // printf("call\n");
            bc_call(context, getParameterFromInstruction(*instructionPointer));
            break;
        case PUSH_FROM_VAR:
            // printf("push_from_var\n");
            bc_push_from_arg(context, args, getParameterFromInstruction(*instructionPointer));
            break;
        case POP_INTO_VAR:
            //  printf("pop_into_var\n");
            bc_pop_into_arg(context, args, getParameterFromInstruction(*instructionPointer));
            break;
        case JMP:
            // printf("jumping %d\n", getParameterFromInstruction(*instructionPointer));
            instructionPointer += getParameterFromInstruction(*instructionPointer);
            break;
        case RETURN:
            //  printf("return\n");
            /* bc_return */
            int16_t result = *(context->stackPointer - 1);
            context->stackPointer = args;
            return result;
            break;
        }
        instructionPointer++;
    }
    return *(context->stackPointer - 1);
}

void
b9PrintStack(ExecutionContext *context)
{

    StackElement *base = context->stack;
    printf("------\n");
    while (base < context->stackPointer) {
        printf("%p: Stack[%ld] = %d\n", base, base - context->stack, *base);
        base++;
    }
    printf("^^^^^^^^^^^^^^^^^\n");
}

int
fib(int n)
{ 
    if (n < 3) return 1;
    return fib (n-1) + fib (n-2);
}

uint64_t *
getJitAddressSlot(Instruction *p)
{
    return (uint64_t *)&p[1];
}

uint64_t
setJitAddressSlot(Instruction *p, uint64_t value)
{
    uint64_t *slotForJitAddress = (uint64_t *)&p[1];
    *getJitAddressSlot(p) = value;
}

bool
hasJITAddress(Instruction *p)
{
    return *getJitAddressSlot(p) != 0;
}

uint64_t
getJitAddress(ExecutionContext *context, int functionIndex)
{
    Instruction *p = context->functions[functionIndex];
    return *getJitAddressSlot(p);
}

void
setJitAddress(ExecutionContext *context, int functionIndex, uint64_t value)
{
    Instruction *p = context->functions[functionIndex];
    setJitAddressSlot(p, value);
}

int
getFunctionCount(ExecutionContext *context)
{
    int functionIndex = 0;
    while(context->functions[functionIndex] != NO_MORE_FUNCTIONS) {
        functionIndex++;
    }
    return functionIndex;
}

void
removeGeneratedCode(ExecutionContext *context, int functionIndex)
{
    setJitAddressSlot(context->functions[functionIndex], 0);
}

void
removeAllGeneratedCode(ExecutionContext *context)
{
    int functionIndex = 0;
    while(context->functions[functionIndex] != NO_MORE_FUNCTIONS) {
        removeGeneratedCode(context, functionIndex);
        functionIndex++;
    }
}

void
generateAllCode(ExecutionContext *context)
{
    int functionIndex = 0;
    while(context->functions[functionIndex] != NO_MORE_FUNCTIONS) {
        generateCode(context->functions[functionIndex], context);
        functionIndex++;
    }
}

void
runFib(ExecutionContext *context, int value) { 
    StackElement result = 0; 
    const char *mode = hasJITAddress(context->functions[1]) ? "JIT" : "Interpreted";
    int validate = fib (value); 
    push(context, value); 
    result = interpret(context, context->functions[1]);
    if (result == validate) { 
        printf("Success: Mode <%s> fib %d returned %d\n", mode, value, result);
    } else { 
        printf("Fail: Mode <%s> fib %d returned %d\n", mode, value, result);
    }
}

void
validateFibResult(ExecutionContext *context) {
    int i;
    for (i=0;i<=12;i++) {
       runFib (context, i);
    }
    generateCode(context->functions[1], context);
    for (i=0;i<=12;i++) {
       runFib (context, i);
    }
}

bool
loadProgram(ExecutionContext *context, const char *programName)
{
    char sharelib[128];
    printf("Loading \"%s\"\n", programName);
    snprintf(sharelib, sizeof(sharelib), "./%s", programName);

    dlerror();
    void *handle = dlopen(sharelib, RTLD_NOW);
    char *error = dlerror();
    if (error) {
        printf("%s\n", error);
        return false;
    }
    Instruction **table = (Instruction **)dlsym(handle, "b9_exported_functions");
    error = dlerror();
    if (error) {
        printf("%s\n", error);
        return false;
    }

    printf("Handle=%p table=%p\n", handle, table);

    context->functions = table;

    return true;
}

StackElement
runProgram(ExecutionContext *context, int functionIndex)
{
    Instruction *func = context->functions[0];

    /* Push random arguments to send to the program */
    int nargs = progArgCount(*func);
    for (int i = 0; i < nargs; i++) {
        int arg = 100 - (i * 10);
        printf("Pushing args %d: %d\n", i, arg);
        push(context, arg);
    }

    return interpret(context, func);
}

int
benchMarkFib(ExecutionContext *context)
{
    /* Load the fib program into the context, and validate that
     * the fib functions return the correct result */
    loadProgram(context, "./bench.so");

    // Validate the our fib is returning the correct results
    // validateFibResult(context);

    StackElement result = 0;

    /* make sure everything is not-jit'd for this initial bench
     * allows you to put examples above, tests etc, and not influence this
     * benchmark compare interpreted vs JIT'd */
     removeAllGeneratedCode(context);

    int LOOP = 200000;
    printf("\nAbout to run %d loops, interpreted\n", LOOP);

    long timeInterp = 0;
    long timeJIT = 0;
    {
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);

        result = interpret(context, context->functions[0]);

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);

        printf("Result is: %d\n", result);
        timeInterp = (tval_result.tv_sec * 1000 + (tval_result.tv_usec / 1000));
    }

    /* Generate code for fib functions */
    // temp, only do fib for now, some issue in loops jit
    generateCode(context->functions[1], context);

    printf("\nAbout to run %d loops, compiled\n", LOOP);

    {
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);

        result = interpret(context, context->functions[0]);

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);

        printf("Result is: %d\n", result);
        timeJIT = (tval_result.tv_sec * 1000 + (tval_result.tv_usec / 1000));
    }

    printf("Time for %d iterations Interp %ld ms JIT %ld ms\n", LOOP, timeInterp, timeJIT);
    printf("JIT speedup = %f\n", timeInterp * 1.0 / timeJIT);

    return 0;
}

/* Main Loop */

int
main(int argc, char *argv[])
{
    b9_jit_init();

    ExecutionContext context;

    char sharelib[128];

    if (argc == 1) {
        printf("No program was passed to b9, Running default benchmark for b9.\n");
        return benchMarkFib(&context);
    }

    for (int i = 1; i < argc; i++) {
        char *name = argv[i];

        if (!loadProgram(&context, name)) {
            return -1;
        }

        printf("  Running Interpreted\n");
        StackElement result = runProgram(&context, 0);
        printf("   result is %ld\n", result);

        printf("  Running JIT\n");
        generateAllCode(&context);
        result = runProgram(&context, 0);
        printf(" result is %ld\n", result);

        return 0;
    }
}
