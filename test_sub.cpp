#include "b9.h"

Instruction test_sub[] = {
    decl(2, 0),
    decl(0, 0),
    decl(0, 0),
    createInstruction(PUSH_FROM_VAR, 0),
    createInstruction(PUSH_FROM_VAR, 1),
    createInstruction(SUB, 0),
    createInstruction(RETURN, 0),
    createInstruction(NO_MORE_BYTECODES, 0)};