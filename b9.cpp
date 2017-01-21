#include "b9.h"

#include <cstring>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
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
    Instruction *program = context->functions[value].program;
    StackElement result = interpret(context, program);
    push(context, result);
}

void
bc_push_from_arg(ExecutionContext *context, StackElement *args, Parameter offset)
{
    push(context, args[offset]);
}

void
bc_pop_into_arg(ExecutionContext *context, StackElement *args, Parameter offset)
{
    args[offset] = pop(context);
}

void
bc_drop(ExecutionContext *context)
{
    pop(context);
}

void
bc_push_constant(ExecutionContext *context, Parameter value)
{
    push(context, value);
}

Parameter
bc_jmp_eq(ExecutionContext *context, Parameter delta)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    if (left == right) {
        return delta;
    }
    return 0;
}

Parameter
bc_jmp_neq(ExecutionContext *context, Parameter delta)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    if (left != right) {
        return delta;
    }
    return 0;
}

Parameter
bc_jmp_gt(ExecutionContext *context, Parameter delta)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    if (left > right) {
        return delta;
    }
    return 0;
}

Parameter
bc_jmp_ge(ExecutionContext *context, Parameter delta)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    if (left >= right) {
        return delta;
    }
    return 0;
}

Parameter
bc_jmp_lt(ExecutionContext *context, Parameter delta)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    if (left < right) {
        return delta;
    }
    return 0;
}

Parameter
bc_jmp_le(ExecutionContext *context, Parameter delta)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    if (left <= right) {
        return delta;
    }
    return 0;
}

void
bc_add(ExecutionContext *context)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    StackElement result = left + right;
    push(context, result);
}

void
bc_sub(ExecutionContext *context)
{
    StackElement right = pop(context);
    StackElement left = pop(context);
    StackElement result = left - right;

    push(context, result);
}

/* ByteCode Interpreter */

StackElement
interpret_0(ExecutionContext *context, Instruction *program) {
 
    return interpret( context,  program);
}
StackElement
interpret_1(ExecutionContext *context, Instruction *program, StackElement p1) {
   
    push (context, p1);
    return interpret( context,  program);
}
StackElement
interpret_2(ExecutionContext *context, Instruction *program, StackElement p1, StackElement p2) {
   
    push (context, p1);
    push (context, p2);
    return interpret( context,  program);
}
StackElement
interpret_3(ExecutionContext *context, Instruction *program, StackElement p1, StackElement p2, StackElement p3 ) {
 
    push (context, p1);
    push (context, p2);
    push (context, p3);
    return interpret( context,  program);
} 

StackElement
interpret(ExecutionContext *context, Instruction *program)
{
    uint64_t *address = (uint64_t *)(&program[1]);
    if (*address) {
        StackElement result = 0;
        if (context->passParameters) {
            int argsCount = progArgCount(*program);
            //printf("about to call jit args %d\n", argsCount);
            switch (argsCount) {
            case 0: {
                JIT_0_args jitedcode = (JIT_0_args)*address;
                result = (*jitedcode)();
            } break;
            case 1: {
                JIT_1_args jitedcode = (JIT_1_args)*address;
                StackElement p1 = pop(context);
                result = (*jitedcode)( p1);
            } break;
            case 2: {
                JIT_2_args jitedcode = (JIT_2_args)*address;
                StackElement p2 = pop(context);
                StackElement p1 = pop(context);
                result = (*jitedcode)(p1, p2);
            } break;
            case 3: {
                JIT_3_args jitedcode = (JIT_3_args)*address;
                StackElement p3 = pop(context);
                StackElement p2 = pop(context);
                StackElement p1 = pop(context);
                result = (*jitedcode)(p1, p2, p3);
            } break;
            default:
                printf("Need to add handlers for more parameters\n");
                break;
            }
        } else {
            Interpret jitedcode = (Interpret)*address;
            result = (*jitedcode)(context, program);
        }
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
            bc_push_constant(context, getParameterFromInstruction(*instructionPointer));
            break;
        case DROP:
            bc_drop(context);
            break;
        case ADD:
            bc_add(context);
            break;
        case SUB:
            bc_sub(context);
            break;
        case JMP:
            instructionPointer += getParameterFromInstruction(*instructionPointer);
            break;
        case JMP_EQ:
            instructionPointer += bc_jmp_eq(context, getParameterFromInstruction(*instructionPointer));
            break;
        case JMP_NEQ:
            instructionPointer += bc_jmp_neq(context, getParameterFromInstruction(*instructionPointer));
            break;
        case JMP_GT:
            instructionPointer += bc_jmp_gt(context, getParameterFromInstruction(*instructionPointer));
            break;
        case JMP_GE:
            instructionPointer += bc_jmp_ge(context, getParameterFromInstruction(*instructionPointer));
            break;
        case JMP_LT:
            instructionPointer += bc_jmp_lt(context, getParameterFromInstruction(*instructionPointer));
            break;
        case JMP_LE:
            instructionPointer += bc_jmp_le(context, getParameterFromInstruction(*instructionPointer));
            break;
        case CALL:
            bc_call(context, getParameterFromInstruction(*instructionPointer));
            break;
        case PUSH_FROM_VAR:
            bc_push_from_arg(context, args, getParameterFromInstruction(*instructionPointer));
            break;
        case POP_INTO_VAR:
            bc_pop_into_arg(context, args, getParameterFromInstruction(*instructionPointer));
            break;
        case RETURN:
            StackElement result = *(context->stackPointer - 1);
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
        printf("%p: Stack[%ld] = %lld\n", base, base - context->stack, *base);
        base++;
    }
    printf("^^^^^^^^^^^^^^^^^\n");
}

uint64_t *
getJitAddressSlot(Instruction *p)
{
    return (uint64_t *)&p[1];
}

void
setJitAddressSlot(Instruction *p, uint64_t value)
{
    uint64_t *slotForJitAddress = getJitAddressSlot(p);
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
    return context->functions[functionIndex].jitAddress; 
}

void
setJitAddress(ExecutionContext *context, int32_t functionIndex, uint64_t value)
{
  context->functions[functionIndex].jitAddress = value; 
  setJitAddressSlot(context->functions[functionIndex].program,   value);
}

int
getFunctionCount(ExecutionContext *context)
{
    int functionIndex = 0;
    while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
        functionIndex++;
    }
    return functionIndex;
}

void
removeGeneratedCode(ExecutionContext *context, int functionIndex)
{
   context->functions[functionIndex].jitAddress = 0;
   setJitAddressSlot(context->functions[functionIndex].program, 0);

}

void
removeAllGeneratedCode(ExecutionContext *context)
{
    int functionIndex = 0;
    while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
        removeGeneratedCode(context, functionIndex);
        functionIndex++;
    }
}

void
generateAllCode(ExecutionContext *context)
{
    int functionIndex = 0;
    while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
        generateCode(context, functionIndex);
        functionIndex++;
    }
}

void
resetContext(ExecutionContext *context)
{
    context->stackPointer = context->stack;
}

bool
loadLibrary(ExecutionContext *context, const char *libraryName)
{
    if (context->library != nullptr) {
        printf("Error loading %s: context already has a library loaded", libraryName);
        return false;
    }

    char sharelib[128];
    if (context->verbose) {
        printf("Loading \"%s\"\n", libraryName);
    }
    snprintf(sharelib, sizeof(sharelib), "./%s", libraryName);

    /* Open the shared object */
    dlerror();
    void *handle = dlopen(sharelib, RTLD_NOW);
    char *error = dlerror();
    if (error) {
        printf("%s\n", error);
        return false;
    }
    context->library = handle;

    /* Get the symbol table */
    struct ExportedFunctionData *table = (struct ExportedFunctionData *)dlsym(handle, "b9_exported_functions");
    error = dlerror();
    if (error) {
        printf("%s\n", error);
        return false;
    }
    context->functions = table; 

    // int functionIndex = 0;
    // while (table[functionIndex].name != NO_MORE_FUNCTIONS) { 
    //     printf ("Name %s, prog %p, jit %p \n", table[functionIndex].name, 
    //         table[functionIndex].program, table[functionIndex].jitAddress);
    //     functionIndex++;
    // } 

    return true;
}

Instruction *
getFunctionAddress(ExecutionContext *context, const char *functionName)
{
    if (context->library == nullptr) {
        printf("Error function %s: context already has a library loaded", functionName);
        return nullptr;
    }

    Instruction *function = (Instruction *)dlsym(context->library, functionName);
    char *error = dlerror();
    if (error) {
        printf("%s\n", error);
        return nullptr;
    }

    return function;
}

StackElement
runFunction(ExecutionContext *context, Instruction *function)
{
    if (context->stackPointer != context->stack) {
        printf("runProgram: Warning Stack not Empty (%ld elements)\n", context->stackPointer - context->stack);
        printf("Possibly a prior run left items on stack, resetting the stack to a clean point.\n");
        resetContext(context);
    }

    /* Push random arguments to send to the program */
    int nargs = progArgCount(*function);
    for (int i = 0; i < nargs; i++) {
        int arg = 100 - (i * 10);
        printf("Pushing args %d: %d\n", i, arg);
        push(context, arg);
    }

    StackElement result = interpret(context, function);

    if (context->stackPointer != context->stack) {
        printf("runProgram: Warning Stack not Empty after running program (%ld elements)\n", context->stackPointer - context->stack);
        resetContext(context);
    }

    return result;
}

StackElement
timeFunction(ExecutionContext *context, Instruction *function, int loopCount, long *runningTime)
{
    struct timeval timeBefore, timeAfter, timeResult;
    StackElement result;

    gettimeofday(&timeBefore, NULL);
    while (loopCount--) {
        result = runFunction(context, function);
    }
    gettimeofday(&timeAfter, NULL);

    timersub(&timeAfter, &timeBefore, &timeResult);
    *runningTime = (timeResult.tv_sec * 1000 + (timeResult.tv_usec / 1000));

    return result;
}

int
parseArguments(ExecutionContext *context, int argc, char *argv[])
{
    char *mainFunction = "b9main";

    /* Command Line Arguments */
    for (int i = 1; i < argc; i++) {
        char *name = argv[i];

        if (!strcmp(name, "-help")) {
            printf("-loop run the program a certain number of times");
            continue;
        }

        if (!strcmp(name, "-loop")) {
            context->loopCount = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-inline")) {
            context->inlineDepthAllowed = atoi(argv[i + 1]); 
            printf ("Allowing Inlining of %d levels \n",  context->inlineDepthAllowed );
            i++;
            continue;
        }

        if (!strcmp(name, "-verbose")) {
            context->verbose = 1;
            continue;
        }

        

        if (!strcmp(name, "-debug")) {
            context->debug++;
            printf("debug is %d \n", context->debug);
            continue;
        }

        if (!strcmp(name, "-directcall")) {
            context->directCall = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-passparameters")) {
            context->passParameters = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-operandstack")) {
            context->operandStack = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (!strcmp(name, "-program")) {
            mainFunction = argv[i + 1];
            i++;
            continue;
        }

        context->name = name;
        continue;
    }
    return 0;
}