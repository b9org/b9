#include "b9.h"
 
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
 
   
static Instruction program[] = {
    decl(0, 1),
    decl(0, 0),
    decl(0, 0), 
    createInstruction(PUSH_CONSTANT, 200000), 
    createInstruction(POP_INTO_VAR, 0), // t = 200000

    // if (t <= 0)  jmp exit
    // loop test
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(PUSH_CONSTANT, 0),
    createInstruction(JMPLE, 8), // SKIP to past the JMP

    createInstruction(PUSH_CONSTANT, 12), // 1
    createInstruction(CALL, 1),           // 2 
    createInstruction(DROP, 0),           // 3

    // t--;
    createInstruction(PUSH_FROM_VAR, 0), // 4
    createInstruction(PUSH_CONSTANT, 1), // 5
    createInstruction(SUB, 0),           // 6
    createInstruction(POP_INTO_VAR, 0),  // 7

    createInstruction(JMP, -11), // 8

    // exit
    createInstruction(PUSH_CONSTANT, 999),
    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};

/* Byte Code Program */
 Instruction *b9_exported_functions[] = {
    program,   // 0
    fib_function,    // 1 
};
 