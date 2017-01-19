#include "b9.h"

Instruction b9main[] = {
	decl (0,8),		// (args,temps)  assume max 8 temps
	decl (0,0),		// 0: space for JIT address
	decl (0,0),		// 1: space for JIT address


	// Local variable a offset 0
	createInstruction(PUSH_CONSTANT,100),	//  number constant 100
	createInstruction(POP_INTO_VAR,0),	// variable a
	// Local variable b offset 1
	createInstruction(PUSH_CONSTANT,10),	//  number constant 10
	createInstruction(POP_INTO_VAR,1),	// variable b
	createInstruction(PUSH_CONSTANT,100),	//  number constant 100
	createInstruction(PUSH_CONSTANT,9),	//  number constant 9
	createInstruction(CALL,1),	// Offset of: 
	createInstruction(DROP,0),	 

	createInstruction(PUSH_CONSTANT,5),	//  number constant 5
	createInstruction(PUSH_CONSTANT,6),	//  number constant 6
	createInstruction(CALL,2),	// Offset of: 
	createInstruction(DROP,0),	 

	createInstruction(PUSH_FROM_VAR,0),	// variable a
	createInstruction(PUSH_FROM_VAR,1),	// variable b
	createInstruction(SUB,0),
	createInstruction(RETURN,0),	//  forced = false
	createInstruction(NO_MORE_BYTECODES, 0)};	//  end of function

Instruction call_sub[] = {
	decl (2,8),		// (args,temps)  assume max 8 temps
	decl (0,0),		// 0: space for JIT address
	decl (0,0),		// 1: space for JIT address


	// Local variable c offset 2
	createInstruction(PUSH_FROM_VAR,0),	// variable a
	createInstruction(PUSH_FROM_VAR,1),	// variable b
	createInstruction(SUB,0),
	createInstruction(POP_INTO_VAR,2),	// variable c
	createInstruction(PUSH_FROM_VAR,2),	// variable c
	createInstruction(RETURN,0),	//  forced = false
	createInstruction(NO_MORE_BYTECODES, 0)};	//  end of function

Instruction call_add[] = {
	decl (2,8),		// (args,temps)  assume max 8 temps
	decl (0,0),		// 0: space for JIT address
	decl (0,0),		// 1: space for JIT address


	// Local variable c offset 2
	createInstruction(PUSH_FROM_VAR,0),	// variable a
	createInstruction(PUSH_FROM_VAR,1),	// variable b
	createInstruction(ADD,0),
	createInstruction(POP_INTO_VAR,2),	// variable c
	createInstruction(PUSH_FROM_VAR,2),	// variable c
	createInstruction(RETURN,0),	//  forced = false
	createInstruction(NO_MORE_BYTECODES, 0)};	//  end of function

	struct ExportedFunctionData b9_exported_functions[] = {
		{  "b9main", b9main, 0},
		{  "call_sub", call_sub, 0},
		{  "call_add", call_add, 0}, 
		{  0,0,0}  
	};

