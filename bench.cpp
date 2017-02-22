#include "b9.h"

Instruction fib_function[] = {
    // one argument, 0 temps
    decl(1, 0),
    decl(0, 0),
    decl(0, 0),

    // if (arg1 < 3) return 1;
    // so converted to jump if 3 <= arg1
    createInstruction(PUSH_CONSTANT, 3),
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(JMP_LE, 2), // SKIP 2 instructions

    // return 1;
    createInstruction(PUSH_CONSTANT, 1), // 1
    createInstruction(RETURN, 0),        // 2

    // return fib (n-1); + fib (n-2);
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(PUSH_CONSTANT, 1),
    createInstruction(SUB, 0),
    createInstruction(CALL, 0), // fib (n-1)

    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(PUSH_CONSTANT, 2),
    createInstruction(SUB, 0),
    createInstruction(CALL, 0), // fib (n-2)

    createInstruction(ADD, 0),

    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};

Instruction b9main[] = {
    decl(0, 2),
    decl(0, 0),
    decl(0, 0),
    createInstruction(PUSH_CONSTANT, 200000), 
    createInstruction(POP_INTO_VAR, 0), // t = 200000

    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(POP_INTO_VAR, 1),  

    // if (t <= 0)  jmp exit
    // loop test
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(PUSH_CONSTANT, 0),
    createInstruction(JMP_LE, 8), // SKIP to past the JMP

    createInstruction(PUSH_CONSTANT, 12), // 1
    createInstruction(CALL, 0),           // 2 
    createInstruction(DROP, 0),           // 3

    // t--;
    createInstruction(PUSH_FROM_VAR, 0), // 4
    createInstruction(PUSH_CONSTANT, 1), // 5
    createInstruction(SUB, 0),           // 6
    createInstruction(POP_INTO_VAR, 0),  // 7

    createInstruction(JMP, -11), // 8

    // exit
    createInstruction(PUSH_FROM_VAR, 1),
    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};

/* Byte Code Program */
struct ExportedFunctionData b9_exported_functions[] = {
    {  "fib_function", fib_function, 0},
    {  "b9main", b9main, 0},
    {  0,0,0}
};

const char *  b9_exported_strings[] = {
0};

PrimitiveData b9_primitives[] = {
    {0, 0}
};

