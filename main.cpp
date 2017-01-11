

#include "b9.h"
#include <dlfcn.h>

#include <sys/time.h>


/* Byte Code Implementations */

void
push(ExecutionContext *context, stack_element_t value)
{
    *context->stackPointer = value;
    ++(context->stackPointer);
}

stack_element_t
pop(ExecutionContext *context)
{
    return *(--context->stackPointer);
}

void
bc_call(ExecutionContext *context, Parameter value)
{
    Instruction *program = context->functions[value];
    // printf("inside call\n");
    stack_element_t result = interpret(context, program);
    // printf("return from  call %d\n", result);

    push(context, result);
}

void
bc_push_from_arg(ExecutionContext *context, stack_element_t *args, Parameter offset)
{
    //  printf("bc_push_from_arg[%d] %d\n", offset, args[offset]);
    push(context, args[offset]);
}

void
bc_pop_into_arg(ExecutionContext *context, stack_element_t *args, Parameter offset)
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
    stack_element_t right = pop(context);
    stack_element_t left = pop(context);
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
    stack_element_t right = pop(context);
    stack_element_t left = pop(context);
    // printf("add right %d left %d\n", right, left);
    stack_element_t result = left + right;

    push(context, result);
}

void
bc_sub(ExecutionContext *context)
{
    // left-right is push(left);push(right); sub
    stack_element_t right = pop(context);
    stack_element_t left = pop(context);
    stack_element_t result = left - right;

    push(context, result);
}

/* ByteCode Interpreter */

stack_element_t
interpret(ExecutionContext *context, Instruction *program)
{
    uint64_t *address = (uint64_t *)(&program[1]);
    if (*address) {
        stack_element_t result = 0;

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
                stack_element_t p1 = pop(context);
                result = (*jitedcode)(context, program, p1);
            }
            break;
            case 2:{
                Interpret_2_args jitedcode = (Interpret_2_args)*address;
                stack_element_t p2 = pop(context);
                stack_element_t p1 = pop(context);
                result = (*jitedcode)(context, program, p1, p2);
            } break;
            case 3:{
                Interpret_3_args jitedcode = (Interpret_3_args)*address;
                stack_element_t p3 = pop(context);
                stack_element_t p2 = pop(context);
                stack_element_t p1 = pop(context);
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
    stack_element_t *args = context->stackPointer - nargs;
    context->stackPointer += tmps; // local storage for temps

    while (*instructionPointer != NO_MORE_BYTECODES) {
        // b9PrintStack(context);
        //printf("about to run %d %d\n", getByteCodeFromInstruction(*instructionPointer), getParameterFromInstruction(*instructionPointer));
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

    stack_element_t *base = context->stack;
    printf("------\n");
    while (base < context->stackPointer) {
        printf("%p: Stack[%ld] = %d\n", base, base - context->stack, *base);
        base++;
    }
    printf("^^^^^^^^^^^^^^^^^\n");
}

int fib (int n) { 
    if (n < 3) return 1;
    return fib (n-1) + fib (n-2);
}

bool hasJITAddress (Instruction *p) {
    uint64_t *slotForJitAddress = (uint64_t *)&p[1];
    return *slotForJitAddress != 0;
}

void runFib (ExecutionContext *context, int value) { 
    stack_element_t result = 0; 
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
void validateFibResult (ExecutionContext *context) {

    int i;
    for (i=0;i<=12;i++) {
       runFib (context, i);
    }
    generateCode(context->functions[1], context);
    for (i=0;i<=12;i++) {
       runFib (context, i);
    }
}
/* Main Loop */

int
main(int argc, char *argv[])
{
    int i;
    b9_jit_init();  

    ExecutionContext context;
    context.functions = 0; // TODO set functions

    for (i = 1; i < argc; i++) {
        char *name = argv[i];
        char sharelib[128];

        printf("Code in a Shared Lib Demo = %s\n", name);
        snprintf(sharelib, sizeof(sharelib), "./%s", name);

        dlerror();
        void *handle = dlopen(sharelib, RTLD_NOW);
        char *error = dlerror();
        if (error) {
            printf("%s\n", error);
            continue;
        }
        Instruction **table = (Instruction **)dlsym(handle, "b9_exported_functions");
        error = dlerror();
        if (error) {
            printf("%s\n", error);
            continue;
        }

        printf("Handle=%p table=%p\n", handle, table);

        Instruction *func = table[0];
        context.functions = table;

        // printf("!!!\n");
        // printf("Context @0=%p, @1 =%p\n", context.functions[0], context.functions[1]);
        // printf("Running %s::%s  %p::%p\n", sharelib, name, handle, func);

        stack_element_t result = 0;
        int nargs = progArgCount(*func);
        for (int i = 0; i < nargs; i++) {
            int arg = 100 - (i * 10);
            printf("Pushing args %d: %d\n", i, arg);
            push(&context, arg);
        }

        /* TODO generate code for all methods */
        generateCode(context.functions[0], &context);
        generateCode(context.functions[1], &context);

        result = interpret(&context, func);
        printf(" result is %ld\n", result);

        return 0;
    }

    validateFibResult(&context);
    stack_element_t result = 0;

#define LOOP 200000

    // make sure everything is not-jit'd for this initial bench
    // allows you to put examples above, tests etc, and not influence this 
    // benchmark compare interpreted vs JIT'd

//     for (int i = 0; i < sizeof(functions) / sizeof(Instruction*); i++) {
//         Instruction *p = functions[i];
//         uint64_t *slotForJitAddress = (uint64_t *)&p[1];
//         *slotForJitAddress = 0;
//    }
// TODO hardcoded to remove the first two jit methods
Instruction *p = context.functions[0];
 uint64_t *slotForJitAddress = (uint64_t *)&p[1];
 *slotForJitAddress = 0;

 p = context.functions[1];
 slotForJitAddress = (uint64_t *)&p[1];
 *slotForJitAddress = 0;

   printf("About to run %d loops, interpreted\n", LOOP);

   long timeInterp = 0;
   long timeJIT = 0;
   do {
       struct timeval tval_before, tval_after, tval_result;
       gettimeofday(&tval_before, NULL); 
        push(&context, LOOP);
        result = interpret(&context, context.functions[0]);
       gettimeofday(&tval_after, NULL);
       timersub(&tval_after, &tval_before, &tval_result);
       printf("Result is: %d\n", result);
       timeInterp = (tval_result.tv_sec*1000 + (tval_result.tv_usec / 1000));
    } while (0);

    generateCode(context.functions[1], &context);  // temp, only do fib for now, some issue in loops jit
    generateCode(context.functions[0], &context);

   do {
       struct timeval tval_before, tval_after, tval_result;
       gettimeofday(&tval_before, NULL);
       push(&context, LOOP);
       result = interpret(&context, context.functions[0]);
       gettimeofday(&tval_after, NULL);
       timersub(&tval_after, &tval_before, &tval_result);
       printf("Result is: %d\n", result); 
       timeJIT =  (tval_result.tv_sec*1000 + (tval_result.tv_usec/1000));

     } while (0);
   
   printf("Time for %d iterations Interp %ld ms JIT %ld ms\n", LOOP, timeInterp, timeJIT);
   printf("JIT speedup = %f\n", timeInterp * 1.0/ timeJIT);

    return 0;
}
