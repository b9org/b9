

#include "b9.h"
#include <dlfcn.h>

#include <sys/time.h>
static Instruction fib_function[] = {
    // one argument, 0 temps
    decl(1, 0),
    decl(0, 0),
    decl(0, 0),

    // if (arg1 < 3) return 1;
    // so converted to jump if 3 <= arg1
    createInstruction(PUSH_CONSTANT, 3),
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(JMPLE, 2), // SKIP 2 instructions

    // return 1;
    createInstruction(PUSH_CONSTANT, 1), // 1
    createInstruction(RETURN, 0),        // 2

    // return fib (n-1); + fib (n-2);
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(PUSH_CONSTANT, 1),
    createInstruction(SUB, 0),
    createInstruction(CALL, 1), // fib (n-1)

    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(PUSH_CONSTANT, 2),
    createInstruction(SUB, 0),
    createInstruction(CALL, 1), // fib (n-2)

    createInstruction(ADD, 0),

    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};




static Instruction test_function[] = {
    decl(0, 0),
    decl(0, 0),
    decl(0, 0),
    createInstruction(PUSH_CONSTANT, 12),
    createInstruction(PUSH_CONSTANT, 99),
    createInstruction(CALL, 2),
    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};



static Instruction main_function[] = {
    decl(0, 0),
    decl(0, 0),
    decl(0, 0),
    createInstruction(PUSH_CONSTANT, 12),
    createInstruction(CALL, 1),
    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};

static Instruction loop_call_fib12_function[] = {
    decl(1, 1),
    decl(0, 0),
    decl(0, 0),
    createInstruction(PUSH_CONSTANT, 666),
    createInstruction(PUSH_CONSTANT, 999),
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(POP_INTO_VAR, 1), // t = 50000

    // if (t <= 0)  jmp exit
    // loop test
    createInstruction(PUSH_FROM_VAR, 1),
    createInstruction(PUSH_CONSTANT, 0),
    createInstruction(JMPLE, 8), // SKIP to past the JMP

    createInstruction(PUSH_CONSTANT, 12), // 1
    createInstruction(CALL, 1),           // 2
    createInstruction(DROP, 0),           // 3

    // t--;
    createInstruction(PUSH_FROM_VAR, 1), // 4
    createInstruction(PUSH_CONSTANT, 1), // 5
    createInstruction(SUB, 0),           // 6
    createInstruction(POP_INTO_VAR, 1),  // 7

    createInstruction(JMP, -11), // 8

    // exit
    createInstruction(PUSH_CONSTANT, 999),
    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};

/* Byte Code Program */
// hack make public so you can direct call from JIT (CALL)
 Instruction *functions[] = {
    main_function,  // call, 0
    fib_function,  // call, 1
    test_function, // call 2
    loop_call_fib12_function};

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
bc_call(ExecutionContext *context, uint16_t value)
{
    Instruction *program = functions[value];
    // printf("inside call\n");
    uint16_t result = interpret(context, program);
    //  printf("return from  call %d\n", result);

    push(context, result);
}

void
bc_push_from_arg(ExecutionContext *context, stack_element_t *args, uint16_t offset)
{
    //  printf("bc_push_from_arg[%d] %d\n", offset, args[offset]);
    push(context, args[offset]);
}

void
bc_pop_into_arg(ExecutionContext *context, stack_element_t *args, uint16_t offset)
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
bc_push_constant(ExecutionContext *context, uint16_t value)
{
    //   printf("bc_push_constant %d\n", value);
    push(context, value);
}

uint16_t
bc_jmple(ExecutionContext *context, uint16_t delta)
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
    return    *slotForJitAddress != 0;
}

void runFib (ExecutionContext *context, int value) { 
    stack_element_t result = 0; 
    const char *mode = hasJITAddress(fib_function) ? "JIT" : "Interpreted";
    int validate = fib (value); 
    push(context, value); 
    result = interpret(context, fib_function);
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
    generateCode(fib_function);
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
    context.functions = functions; 

    for (i=1;i<argc;i++) { 
        char sharelib[128];
        char func_name[128];
        char *name = argv[i];
        printf ("Code in a Shared Lib Demo = %s\n", name);
        snprintf(sharelib, sizeof(sharelib), "%s.so", name); 
        snprintf(func_name, sizeof(func_name), "_%s", name); 
        void * handle = dlopen (sharelib, RTLD_NOW); 
        Instruction* func;
        func = (Instruction*) dlsym(handle, name); 
        printf ("Running %s::%s  %p::%p\n", sharelib, name, handle, func);
        if (func == 0) {  
            func = (Instruction*) dlsym(handle, func_name); 
            printf ("Running %s::%s  %p::%p\n", sharelib, func_name, handle, func);
        }
        if (func != 0) { 
            stack_element_t result = 0;
           int nargs = progArgCount(*func);
           for (int i=0;i<nargs;i++){
            int arg = 100 - (i*10);
            printf ("Pushing args %d: %d\n", i, arg);
            push(&context, arg); 
           }
           result = interpret(&context, (Instruction*) func);
            printf (" result is %ld\n", result); 
            return 0;
        }
    }
    
    validateFibResult(&context);
    stack_element_t result = 0;

#define LOOP 200000

    // make sure everything is not-jit'd for this initial bench
    // allows you to put examples above, tests etc, and not influence this 
    // benchmark compare interpreted vs JIT'd
    for (int i = 0; i < sizeof(functions) / sizeof(Instruction*); i++) {
        Instruction *p = functions[i];
        uint64_t *slotForJitAddress = (uint64_t *)&p[1];
        *slotForJitAddress = 0;
   }


   printf("About to run %d loops, interpreted\n", LOOP);

   long timeInterp = 0;
   long timeJIT = 0;
   do {
       struct timeval tval_before, tval_after, tval_result;
       gettimeofday(&tval_before, NULL); 
        push(&context, LOOP);
        result = interpret(&context, loop_call_fib12_function);
       gettimeofday(&tval_after, NULL);
       timersub(&tval_after, &tval_before, &tval_result);
       printf("Result is: %d\n", result);
       timeInterp = (tval_result.tv_sec*1000 + (tval_result.tv_usec / 1000));
    } while (0);

   for (int i = 0; i < sizeof(functions) / sizeof(Instruction*); i++) {
     //  generateCode(functions[i]);   // temp, do not compile all 
   }

    generateCode(fib_function);  // temp, only do fib for now, some issue in loops jit
    generateCode(loop_call_fib12_function);

   do {
       struct timeval tval_before, tval_after, tval_result;
       gettimeofday(&tval_before, NULL);
       push(&context, LOOP);
       result = interpret(&context, loop_call_fib12_function);
       gettimeofday(&tval_after, NULL);
       timersub(&tval_after, &tval_before, &tval_result);
       printf("Result is: %d\n", result); 
       timeJIT =  (tval_result.tv_sec*1000 + (tval_result.tv_usec/1000));

     } while (0);
   
   printf("Time for %d iterations Interp %ld ms JIT %ld ms\n", LOOP, timeInterp, timeJIT);
   printf("JIT speedup = %f\n", timeInterp * 1.0/ timeJIT);

    return 0;
}
