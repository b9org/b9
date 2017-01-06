#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstddef>

/* Bytecodes */

/**
 * Each bytecode is 32 bytes:
 * first 16 bytes are the OpCode
 * second 16 bytes may be an argument to the opcode
 */

typedef uint16_t ByteCode;
typedef uint16_t Parameter;
typedef uint32_t Instruction;

#define NO_MORE_BYTECODES 0x0
#define PUSH_CONSTANT 0X1
#define DROP 0x2
#define PUSH_FROM_VAR 0x3
#define POP_INTO_VAR 4
#define SUB 5
#define ADD 6
#define CALL 7
#define RETURN 8
#define JMPLE 9


/* VM State */

class ExecutionContext {
    public:
    ExecutionContext () :
    stackPointer(this->stack)
    {
        std::memset(stack, 0, sizeof(stack));
    }

    uint16_t stack[1000];
    uint16_t *stackPointer;
};

uint16_t interpret(ExecutionContext *context, Instruction *program);

constexpr ByteCode
getByteCodeFromInstruction (Instruction instruction)
{
    return (instruction >> 16);
}

constexpr Parameter
getParameterFromInstruction (Instruction instruction)
{
    return (instruction & 0xFFFF);
}

constexpr Instruction
createInstruction(ByteCode byteCode, Parameter parameter)
{
    return byteCode << 16 | parameter;
}

#define decl(argCount, tmpCount) (argCount << 16 | tmpCount)
#define progArgCount(a) (a >> 16)
#define progTmpCount(a) (a & 0xFFFF)

static Instruction fib_function [] = {
     decl(1, 0),

     createInstruction(PUSH_CONSTANT, 3),
     createInstruction(PUSH_FROM_VAR, 0),
     createInstruction(JMPLE, 3),
     createInstruction(PUSH_CONSTANT, 1),
     createInstruction(RETURN, 0),
     createInstruction(PUSH_FROM_VAR, 0),
     createInstruction(PUSH_CONSTANT, 1),
     createInstruction(SUB, 0),
     createInstruction(CALL, 1),

     createInstruction(PUSH_FROM_VAR, 0),
     createInstruction(PUSH_CONSTANT, 2),
     createInstruction(SUB, 0),
     createInstruction(CALL, 1),

     createInstruction(ADD, 0),

     createInstruction(RETURN, 0),
     createInstruction(NO_MORE_BYTECODES, 0)
};

static Instruction main_function [] =
{
    decl(0, 0),
    createInstruction(PUSH_CONSTANT, 12),
    createInstruction(CALL, 1),
    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)
};

/* Byte Code Program */
static Instruction *functions [] = {
    main_function,
    fib_function
};
/* Byte Code Implementations */

#define PUSH_FROM_VAR 0x3
#define POP_INTO_VAR 4

#define CALL 7

void
push (ExecutionContext *context, uint16_t value)
{
    *context->stackPointer = value;
    ++(context->stackPointer);
}

uint16_t
pop (ExecutionContext *context)
{
    return *(--context->stackPointer);
}

void
bc_call(ExecutionContext *context,  uint16_t value)
{
    Instruction *program = functions [value];
    printf("inside call");
    uint16_t result = interpret(context, program);
    printf("return from  call %d\n" ,result);

    push (context, result);
}


void
bc_push_from_arg(ExecutionContext *context, uint16_t *args, uint16_t offset)
{
    push(context, args[offset]);
}

void
bc_pop_into_arg(ExecutionContext *context, uint16_t *args, uint16_t offset)
{
    args[offset] = pop(context);
}

void
bc_drop(ExecutionContext *context)
{
    pop(context);
}

void
bc_push_constant (ExecutionContext *context, uint16_t value)
{
    push(context, value);
}

uint16_t
bc_jmple(ExecutionContext *context, uint16_t delta)
{
    uint16_t operandB = pop(context);
    uint16_t operandA = pop(context);

    printf("jmple operandA %d, operandB %d\n", operandA, operandB);

    if (operandA <= operandB) {
        return delta - 1;
    }
    return 0;
}


void
bc_add (ExecutionContext *context)
{

    uint16_t operandA = pop(context);
    uint16_t operandB = pop(context);
    printf("add operandA %d operandB %d\n", operandA, operandB);
    uint16_t result = operandA + operandB;

    push(context, result);

}

void
bc_sub (ExecutionContext *context)
{

    uint16_t operandB = pop(context);
    uint16_t operandA = pop(context);
    uint16_t result = operandA - operandB;

    push(context, result);

}



/* ByteCode Interpreter */

uint16_t interpret(ExecutionContext *context, Instruction *program)
{
    int nargs = progArgCount(*program);
    int tmps = progTmpCount(*program);

    printf("Prog Arg Count %d, tmp count %d", nargs, tmps);

    Instruction *instructionPointer = program + 1;
    uint16_t *args = context->stackPointer - nargs;

    while (instructionPointer != NO_MORE_BYTECODES)
    {
        switch (getByteCodeFromInstruction(*instructionPointer))
        {
            case PUSH_CONSTANT :
                printf("push\n");
                bc_push_constant(context, getParameterFromInstruction(*instructionPointer));
                break;
            case DROP :
                printf("drop\n");
                bc_drop(context);
                break;
            case ADD :
                printf("add\n");
                bc_add(context);
                break;
            case SUB :
                printf("sub\n");
                bc_sub(context);
                break;
            case JMPLE :
                printf("jmple\n");
                instructionPointer += bc_jmple(context, getParameterFromInstruction(*instructionPointer));
                break;
            case CALL :
                printf ("call\n");
                bc_call(context, getParameterFromInstruction(*instructionPointer));
                break;
            case PUSH_FROM_VAR :
                printf ("poush_from_var\n");
                bc_push_from_arg(context, args, getParameterFromInstruction(*instructionPointer));
                break;
            case POP_INTO_VAR :
                printf ("pop_into_var\n");
                bc_pop_into_arg(context, args, getParameterFromInstruction(*instructionPointer));
                break;
            case RETURN :
                printf("return\n");
                /* bc_return */
                int16_t result = *(context->stackPointer - 1);
                context->stackPointer = args;
                return result;
                break;
        }
        instructionPointer++;
    }
    return *(context->stackPointer-1);
}



/* Main Loop */

int
main ()
{
    Instruction *programToRun = main_function;
    ExecutionContext context;

    uint16_t result = interpret(&context, programToRun);

    printf("Program result is: %d", result);

    return 0;
}
