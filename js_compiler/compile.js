/*************************************
CONVERT B9 PORCELAIN TO BINARY MODULE 
**************************************/

// syntax is <in> <out>

var esprima = require('esprima');
var fs = require('fs');

function outputUInt32(out, value) {
	var buf = Buffer.alloc(4);
	buf.writeUInt32LE(value, 0);
	fs.writeSync(out, buf, 0, 4);
}

function outputString(out, string) {
	outputUInt32(out, string.length);
	fs.writeSync(out, string);
}

var PrimitiveCode = Object.freeze({
	"print_string": 0,
	"print_number": 1,
	"print_stack": 2
});

var OperatorCode = Object.freeze({
	"END_SECTION": 0,
	"FUNCTION_CALL": 1,
	"FUNCTION_RETURN": 2,
	"PRIMITIVE_CALL": 3,
	"JMP": 4,
	"DUPLICATE": 5,
	"DROP": 6,
	"PUSH_FROM_VAR": 7,
	"POP_INTO_VAR": 8,
	"INT_ADD": 9,
	"INT_SUB": 10,
	"INT_MUL": 11,
	"INT_DIV": 12,
	"INT_PUSH_CONSTANT": 13,
	"INT_NOT": 14,
	"INT_JMP_EQ": 15,
	"INT_JMP_NEQ": 16,
	"INT_JMP_GT": 17,
	"INT_JMP_GE": 18,
	"INT_JMP_LT": 19,
	"INT_JMP_LE": 20,
	"STR_PUSH_CONSTANT": 21,
	"STR_JMP_EQ": 22,
	"STR_JMP_NEQ": 23,
	"PUSH_FROM_ARG": 24,
	"POP_INTO_ARG": 25
});

/// Binary comparison operators converted to jump instructions
JumpOperator = Object.freeze({
	"==": "INT_JMP_EQ",
	"!=": "INT_JMP_NEQ",
	"<=": "INT_JMP_LE",
	">=": "INT_JMP_GE",
	"<": "INT_JMP_LT",
	">": "INT_JMP_GT"
});

/// Map binary comparison operators to jump instructions that invert the condition.
/// as an example, the if statement handler will use this table to invert a comparison, and jump over the if-true block.
var NegJumpOperator = Object.freeze({
	"==": "INT_JMP_NEQ",
	"!=": "INT_JMP_EQ",
	"<=": "INT_JMP_GT",
	">=": "INT_JMP_LT",
	"<": "INT_JMP_GE",
	">": "INT_JMP_LE"
});

/// Map a++ style operators to b9 operator codes.
var UpdateOperator = Object.freeze({
	"++": "INT_ADD",
	"--": "INT_SUB"
});

/// A B9 Instruction -- operator and operand pair.
var Instruction = function (operator, operand) {

	this.operator = operator;
	this.operand = operand;

	/// Output this instruction as an encoded uint32_t (little endian).
	this.output = function (out) {
		var encoded = (OperatorCode[this.operator] << 24);
		if (this.operand) {
			encoded |= this.operand & 0x00FFFFFF;
		}
		encoded &= 0xFFFFFFFF;
		outputUInt32(out, encoded);
	}
};

var SymbolTable = function () {

	this.next = 0;
	this.map = {};

	/// Look up a symbol without interning.
	this.lookup = function (symbol) {
		return this.map[symbol];
	}

	/// Look up a symbol, and intern if not found.
	this.get = function (symbol) {
		var id = this.lookup(symbol);
		if (id == undefined) {
			id = this.next;
			this.next += 1;
			this.map[symbol] = id;
		}
		return id;
	}

	/// callback(name, id)
	this.forEach = function (callback) {
		var me = this;
		for (symbol in this.map) {
			callback(symbol, me.map[symbol]);
		}
	}
}

function GlobalContext() {
	// index 0 is reserved for the script body!
	this.nextFunctionId = 1;
}

function BlockEntry(globalContext) { //currently unused
	
	this.functionTable = {};
	this.localTable = new SymbolTable();

	this.lookup = function (symbol) {

		var id = undefined;

		id = this.localTable.lookup(symbol);
		if (id !== undefined) {
			return { type: "local", id: id };
		}

		id = this.functionTable[symbol];
		if (id !== undefined) {
			return { type: "function", id: id };
		}
		return undefined;
	}

	this.defineFunction = function (symbol) {
		this.functionTable[symbol] = globalContext.nextFunctionId;
		globalContext.nextFunctionId++;
		return this.lookup(symbol);
	}

	this.defineLocal = function (symbol) {
		return this.localTable.get(symbol);
	}
};

function BlockContext(globalContext) { //currently unused

	this.stack = [];

	this.push = function (entry) {
		if (entry == undefined) {
			throw new Error("Entering undefined block entry");
		}
		this.stack.push(entry);
		return entry;
	}

	this.pop = function () {
		return this.stack.pop();
	}

	this.defineFunction = function (symbol) {
		return this.stack[this.stack.length - 1].defineFunction(symbol);
	}

	this.lookup = function (symbol) {
		var i = 0;
		for (; i < this.stack.length; i++) {
			var result = this.stack[this.stack.length - i - 1].lookup(symbol);
			if (result != undefined) {
				return result;
			}
		}
		return undefined;
	}
}

function FunctionEntry(globalContext) {

	this.parameterTable = new SymbolTable();
	this.localTable = new SymbolTable();
	this.functionTable = {};

	this.defineParameter = function (symbol) {
		return this.parameterTable.get(symbol);
	}

	this.defineFunction = function (symbol) {
		this.functionTable[symbol] = globalContext.nextFunctionId;
		globalContext.nextFunctionId++;
		return this.lookup(symbol);
	}

	this.defineLocal = function (symbol) {
		return this.localTable.get(symbol);
	}

	this.lookup = function (symbol) {

		var id = undefined;

		id = this.localTable.lookup(symbol);
		if (id !== undefined) {
			return { type: "local", id: id };
		}

		id = this.parameterTable.lookup(symbol);
		if (id != undefined){
			return {type: "parameter", id: id};
		}

		id = this.functionTable[symbol];
		if (id !== undefined) {
			return { type: "function", id: id };
		}
		return undefined;
	}
};

function FunctionContext() {
	this.stack = [];

	this.currentFunction = function() {
		return this.stack[this.stack.length - 1];
	}

	this.enterFunction = function (functionEntry) {
		if (functionEntry == undefined) {
			throw new Error("Entering undefined function entry");
		}
		this.stack.push(functionEntry);
	}

	this.exitFunction = function () {
		this.stack.pop();
	}

	this.lookup = function (symbol) {
		var i = 0;
		for (; i < this.stack.length; i++) {
			var result = this.stack[this.stack.length - i - 1].lookup(symbol);
			if (result != undefined) {
				result.hops = i;
				return result;
			}
		}
		throw new Error("could not find symbol: " + symbol + " in:\n" + JSON.stringify(this.stack, null, 2));
		return undefined;
	}
};

var LabelTable = function () {

	this.table = [];

	this.create = function () {
		return this.table.push(undefined);
	}

	this.place = function (label, offset) {
		this.table[label] = offset;
	}

	this.createAndPlace = function (offset) {
		id = this.make();
		this.place(id, offset);
		return id;
	}

	this.instructionIndex = function (label) {
		return this.table[label];
	}

};

/// A section of code. CodeBody has information about args and registers, but no information about indexes or it's name.
/// The name-to-index mapping is managed externally, by the module's FunctionTable. Eventually, the args and registers
/// might move to a lexical environment, and this will become a simple bytecode array.
function FunctionDefinition(outer, name, index) {
	this.name = name;
	this.index = index;
	this.args = new SymbolTable();
	this.regs = new SymbolTable();
	this.labels = new LabelTable();
	this.instructions = [];

	/// Resolve the label to a relative offset.
	this.resolveLabel = function (label, fromIndex) {
		var target = this.labels.instructionIndex(label);
		if (target == undefined) {
			throw "Encountered label " + label;
		}
		return target - fromIndex - 1;
	};

	this.resolve = function (module) {
		for (var index = 0; index < this.instructions.length; index++) {
			var instruction = this.instructions[index];
			switch (instruction.operator) {
				case "JMP":
				case "INT_JMP_EQ":
				case "INT_JMP_NEQ":
				case "INT_JMP_GT":
				case "INT_JMP_GE":
				case "INT_JMP_LT":
				case "INT_JMP_LE":
					// the label id is stuffed in the operand.
					// translate the label to a relative offset.
					instruction.operand = this.resolveLabel(instruction.operand, index);
			}
		}
	}

	this.output = function (out) {
		// note that name and index are output by the module.
		outputUInt32(out, this.args.next);
		outputUInt32(out, this.regs.next);
		this.instructions.forEach(function (instruction) {
			instruction.output(out);
		})
	};

	this.pushInstruction = function (instruction) {
		this.instructions.push(instruction);
	};

	this.lastInstruction = function () {
		return this.instructions[this.instructions.length - 1];
	}

	this.localIndex = function (name) {
		var index = this.regs.lookup(name);
		if (index) {
			return index + this.args.next;
		}
		return this.args.lookup(name);
	}

	this.placeLabel = function (label) {
		this.labels.place(label, this.instructions.length);
	}

	this.createLabel = function () {
		return this.labels.create();
	}

};

function Module() {
	this.resolved = false;
	this.functions = [];
	this.strings = new SymbolTable();

	/// After the module has been entirely built up, resolve any undefined references.
	this.resolve = function () {
		var me = this;
		this.functions.forEach(function (body) {
			if (!body) {
				throw "Undefined function reference: " + name;
			}
			body.resolve(me);
		});
		this.resolved = true;
	}

	/// Output this module, in binary format. The Module must have been resolved.
	this.output = function (out) {
		if (!this.resolved) {
			throw "Module must be resolved before output."
		}
		this.outputHeader(out);
		this.outputFunctionSection(out);
		this.outputStringSection(out);
	}

	//
	// INTERNAL
	//

	this.outputHeader = function (out) {
		/// Raw magic bytes
		fs.writeSync(out, 'b9module');
	};

	/// internal
	this.outputFunctionSection = function (out) {
		var me = this;
		outputUInt32(out, 1); // the section code.
		outputUInt32(out, this.functions.length);
		for (var i = 0; i < this.functions.length; ++i) {
			var func = this.functions[i];
			outputString(out, func.name);
			outputUInt32(out, func.index);
			func.output(out);
		}
	}

	this.outputStringSection = function (out) {
		outputUInt32(out, 2); // the section code.
		outputUInt32(out, this.strings.next);
		this.strings.forEach(function (string, id) {
			outputString(out, string);
		});
	}
};

function FirstPassCodeGen() {

	this.globalContext = new GlobalContext();
	this.functionContext = new FunctionContext(this.globalContext);
	this.module = undefined;
	this.func = undefined;

	this.compile = function (syntax) {
		this.module = new Module();
		var func = new FunctionDefinition(null, "<body>", 0); // top level
		this.module.functions.push(func);
		this.augmentAST(syntax);
		this.functionContext.enterFunction(syntax.functionEntry);
		this.handleBody(func, syntax.body);
		func.instructions.push(new Instruction("INT_PUSH_CONSTANT", 0));
		func.instructions.push(new Instruction("FUNCTION_RETURN", 0));
		func.instructions.push(new Instruction("END_SECTION", 0)); //end of top level function
		this.functionContext.exitFunction(); 
		return this.module;
	};

	this.augmentAST = function (syntax) {
		var global = new GlobalContext();
		
		syntax.functionEntry = new FunctionEntry(global);
	
		var stack = [{ todo: Object.assign([], syntax.body), func: syntax.functionEntry }];

		while (stack.length != 0) {
			var work = stack[stack.length - 1];

			if (work.todo.length == 0) {
				stack.pop();
				continue;
			}

			var func = work.func;
			var node = work.todo.shift();

			switch (node.type) {
				case "ForStatement":
					node.init.functionEntry = func;
					if (node.init.type == "VariableDeclaration"){
						node.init.declarations.forEach(function(d){
							node.init.functionEntry.defineLocal(d.id.name);
						});
					}
					node.update.functionEntry = func;
					node.body.functionEntry = func;
					work = { todo: Object.assign([], node.body), func: func};
					stack.push(work);
					break;
				case "WhileStatement":
					node.body.functionEntry = func;
					work = { todo: Object.assign([], node.body), func: func};
					stack.push(work);
					break;
				case "IfStatement":
					node.test.functionEntry = func;
					if (node.alternate != null){
						node.alternate.functionEntry = func;
					}
					node.consequent.blockEntry = func;
					work = { todo: Object.assign([],node.consequent.body), func: func};
					stack.push(work);
					break;
				case "FunctionDeclaration":
					func.defineFunction(node.id.name);
					node.functionEntry = new FunctionEntry(global);
				
					node.params.forEach(function (param) {
						node.functionEntry.defineParameter(param.name);
					});

					node.body.functionEntry = node.functionEntry;
					work = { todo: Object.assign([], node.body.body), func: node.functionEntry };
					stack.push(work);
				break;
				case "BlockStatement":
					node.functionEntry = func;
					work = { todo: Object.assign([], node.body), func: func };
					stack.push(work);
					break;
				case "VariableDeclaration":
					node.declarations.forEach(function (d) {
						func.defineLocal(d.id.name);
					});
					break;
				default:
					break;
			}
		}
	}

	this.handleBody = function (func, body) {
		var me = this;
		body.forEach(function (element) {
			me.handle(func, element);
		});
	}

	this.handle = function (func, element) {
		if (!func || !element) {
			throw new Error("Invalid func/element passed to syntax handler");
		}
		this.getHandler(element.type).call(this, func, element);
	};

	this.getHandler = function (elementType) {
		var name = "handle" + elementType;
		var handler = this[name];
		if (!handler) {
			throw "No handler for type " + elementType;
		}
		return handler;
	}

	/* STACK OPERATIONS */

	this.emitPushConstant = function (func, constant) {
		if (this.isNumber(constant)) {
			func.instructions.push(new Instruction("INT_PUSH_CONSTANT", constant));
		}
		else if (this.isString(constant)) {
			var id = this.module.strings.get(constant);
			func.instructions.push(new Instruction("STR_PUSH_CONSTANT", id));
		}
		// func.updateStackCount(1);
		else {
			throw "Unsupported constant/literal encountered: " + constant;
		}
	}

	this.emitPushFromVar = function (func, name) {
		var symbol = this.functionContext.lookup(name);

		// if (symbol.hops > 0) {
		// 	throw new Error("Cannot access across scopes: " + symbol);
		// }
		if (symbol.type == "parameter") {
			func.instructions.push(new Instruction("PUSH_FROM_ARG", symbol.id));
		}
		else if (symbol.type == "local"){
			func.instructions.push(new Instruction("PUSH_FROM_VAR", symbol.id));
		}
		else{
			throw new Error("Cannot read from lon-local/parameter: " + symbol);
		}
	}

	this.emitPopIntoVar = function (func, name) {
		var symbol = this.functionContext.lookup(name);
		// if (symbol.hops > 0) {
		// 	throw new Error("Cannot access across scopes: " + symbol);
		// }
		if (symbol.type == "parameter") {
			func.instructions.push(new Instruction("POP_INTO_ARG", symbol.id));
		}
		else if (symbol.type == "local"){
			func.instructions.push(new Instruction("POP_INTO_VAR", symbol.id));
		}
		else{
			throw new Error("Cannot store to non-local/parameter: " + symbol);
		}
	}

	this.handleFunctionDeclaration = function (outerFunc, declaration) {
		this.functionContext.enterFunction(declaration.functionEntry);
		var symbol = this.functionContext.lookup(declaration.id.name);
		var inner = new FunctionDefinition(outerFunc, declaration.id.name, symbol.id);
		this.module.functions.push(inner);
		declaration.params.forEach(function (param) {
			inner.args.get(param.name);
		});
		inner.nargs = declaration.params.length;
		this.handle(inner, declaration.body);

		/// this discards the result of the last expression
		if (inner.instructions[inner.instructions.length - 1].operator != "FUNCTION_RETURN") {
			inner.instructions.push(new Instruction("INT_PUSH_CONSTANT", 0));
			inner.instructions.push(new Instruction("FUNCTION_RETURN", 0));
		}

		inner.instructions.push(new Instruction("END_SECTION", 0));
		this.functionContext.exitFunction();
	};

	this.handleAssignmentExpression = function (func, expression) {
		var AssignmentOperatorCode = Object.freeze({
			"+=": "INT_ADD",
			"-=": "INT_SUB",
			"/=": "INT_DIV",
			"*=": "INT_MUL"
		});

		if (expression.left.type == "Identifier") {
			var operator = AssignmentOperatorCode[expression.operator];
			if (operator) {
				this.handle(func, expression.left); // extra left
				this.handle(func, expression.right);
				func.instructions.push(new Instruction(operator, 0));
			} else {
				expression.right.needResult = true;
				this.handle(func, expression.right);
			}
			func.instructions.push(new Instruction("DUPLICATE"));
			this.emitPopIntoVar(func, expression.left.name);

			if (expression.isParameter == true) {
				this.emitPushFromVar(func, expression.left.name);
			}
			return;
		}
		this.handle(func, expression.right);
	};

	this.handleVariableDeclaration = function (func, declaration) {
		/// composed of potentially multiple variables.
		this.handleBody(func, declaration.declarations);
	}

	this.handleVariableDeclarator = function (func, declarator) {
		var id = func.regs.get(declarator.id.name);
		if (declarator.init) {
			this.handle(func, declarator.init);
			this.emitPopIntoVar(func, declarator.id.name);
		}
	};

	this.handleExpressionStatement = function (func, statement) {
		this.handle(func, statement.expression);
		func.instructions.push(new Instruction("DROP"));
	};

	// Iterate expressions array
	this.handleSequenceExpression = function (func, sequence) {
		var expressions = sequence.expressions;
		var droplast = !decl.isParameter;

		for (expression in sequence.expressions.slice(0, -1)) {
			this.handle(func, expression);
			func.instructions.push(new Instruction("DROP"));
		}

		var last = sequence.expressions.slice(-1)[0];
		this.handle(last);

		if (!sequence.isParameter) {
			func.instructions.push(new Instruction("DROP"));
		}
	};

	this.handleUnaryExpression = function (func, decl) {
		if (decl.operator == "-" && decl.argument.type == "Literal") {
			func.instructions.push(new Instruction("INT_PUSH_CONSTANT", - decl.argument.value));
			// this.currentFunction.updateStackCount(1);
			return;
		}
		if (decl.operator == "+") {
			this.handle(func, decl.argument);
			return;
		}
		if (decl.operator == "-") {
			this.pushconstant(0);
			func.instructions.push(new Instruction("INT_PUSH_CONSTANT", 0));
			this.handle(func, decl.argument);
			func.instructions.push(new Instruction("INT_SUB"));
			return;
		}
		if (decl.operator == "!") {
			this.handle(func, decl.argument);
			func.instructions.push(new Instruction("INT_NOT"));
			return;
		}
		throw Error("No Handler for Type: " + decl);
	};


	this.handleBinaryExpression = function (func, decl) {
		this.handle(func, decl.left);
		this.handle(func, decl.right);
		if (decl.operator == "-") {
			func.instructions.push(new Instruction("INT_SUB"));
		}
		else if (decl.operator == "+") {
			func.instructions.push(new Instruction("INT_ADD"));
		}
		else if (decl.operator == "*") {
			func.instructions.push(new Instruction("INT_MUL"));
		}
		else if (decl.operator == "/") {
			func.instructions.push(new Instruction("INT_DIV"));
		}
		else {
			// TODO: Support comparison operators.
			// Note: Comparison operations are only handled in if-statements
			// if-statements will use the operator to emit a specialized jmp-compare instruction
			throw "This operator is not being handled" + decl.operator;
		}
	};

	this.handleBlockStatement = function (func, decl) {
		this.handleBody(func, decl.body);
		// TODO: Missing drop?
	}

	this.handleUpdateExpression = function (func, expression) {
		if (expression.argument.type != "Identifier") {
			throw Error("Invalid target of update expression");
		}

		this.emitPushFromVar(func, expression.argument.name);
		// postfix operator leaves the original value on the stack
		if (!expression.prefix) {
			func.instructions.push(new Instruction("DUPLICATE"));
		}
		func.instructions.push(new Instruction("INT_PUSH_CONSTANT", 1));
		func.instructions.push(new Instruction(UpdateOperator[expression.operator]));
		// prefix leaves the new value on the stack.
		if (expression.prefix) {
			func.instructions.push(new Instruction("DUP"));
		}
		this.emitPopIntoVar(func, expression.argument.name);
	};

	this.handleForStatement = function (func, statement) {
		this.handle(func, statement.init);
		var testLabel = func.labels.create();
		var exitLabel = func.labels.create();
		func.placeLabel(testLabel);
		var comparator = this.emitTest(func, statement.test);
		func.instructions.push(new Instruction(NegJumpOperator[comparator], exitLabel));
		this.handle(func, statement.body);
		this.handle(func, statement.update);
		func.instructions.push(new Instruction("DROP"));
		func.instructions.push(new Instruction("JMP", testLabel));
		func.placeLabel(exitLabel);
	};

	this.handleCallExpression = function (func, expression) {
		// Set up arguments for call

		if (expression.callee.type != "Identifier") {
			throw "Only handles named functions";
		}

		if (!expression.callee.name) {
			throw "Trying to compile call to function with no name";
		}

		/// TODO: Only supports calling functions by name
		if (expression.callee.name == "b9_primitive") {
			this.emitPrimitiveCall(func, expression)
		}
		else {
			this.emitFunctionCall(func, expression);
		}
	}

	this.handleReturnStatement = function (func, decl) {
		if (decl.argument) {
			this.handle(func, decl.argument);
		}
		else {
			func.instructions.push(new Instruction("INT_PUSH_CONSTANT", 0));
		}

		func.instructions.push(new Instruction("FUNCTION_RETURN"));
	};

	this.emitFunctionCall = function (func, expression) {
		var symbol = this.functionContext.lookup(expression.callee.name);
		if (symbol.type != "function") throw Error("Target not a direct function call: " + JSON.stringify(symbol));
		this.handleBody(func, expression.arguments);
		func.instructions.push(new Instruction("FUNCTION_CALL", symbol.id));
	}

	this.emitPrimitiveCall = function (func, expression) {
		/// The first argument is a "phantom" argument that tells us the primitive code.
		/// It's not compiled as an expression.
		var code = PrimitiveCode[expression.arguments[0].value];

		var args = expression.arguments.slice(1);
		args.forEach(function (element) {
			element.isParameter = true;
		});
		this.handleBody(func, args);
		func.instructions.push(new Instruction("PRIMITIVE_CALL", code));
		return true;
	};

	/* HANDLE JUMPS AND LABELS */

	this.isNumber = function isNumber(num) {
		return typeof num == "number";
	};

	this.isString = function isString(num) {
		return typeof num == 'string';
	};

	this.getStringIndex = function (id) {
		return this.strings.lookup(string);
		if (this.strings[id] != undefined) {
			return this.strings[id];
		} else {
			this.strings[id] = this.nextStringIndex++;
			return this.strings[id];
		}
	};

	this.handleLiteral = function (func, literal) {
		this.emitPushConstant(func, literal.value);
	}

	this.handleIdentifier = function (func, identifier) {
		this.emitPushFromVar(func, identifier.name);
	};

	/// Emit a test statement.
	/// The test statement returns the comparator 
	this.emitTest = function (func, test) {
		var op = undefined;
		if (test.type == "BinaryExpression") {
			// Binary expressions compile to specialized JMP operations
			this.handle(func, test.left);
			this.handle(func, test.right);
			op = test.operator;
		} else {
			// Unary expressions always compile to a comparison with false.
			this.handle(func, test.body);
			func.instructions.push(new Instruction("INT_PUSH_CONSTANT", 0));
			op = "==";
		}
		return op;
	}

	this.handleIfStatement = function (func, statement) {
		var alternateLabel = func.labels.create();
		var endLabel = func.labels.create();
		comparator = this.emitTest(func, statement.test);
		if (statement.alternate) {
			func.instructions.push(new Instruction(NegJumpOperator[comparator], alternateLabel));
		} else {
			func.instructions.push(new Instruction(NegJumpOperator[comparator], endLabel));
		}
		this.handle(func, statement.consequent);
		if (statement.alternate) {
			if (func.lastInstruction().operator != "RETURN") {
				func.instructions.push(new Instruction("JMP", endLabel));
			}
			func.placeLabel(alternateLabel);
			this.handle(func, statement.alternate);
		}
		func.placeLabel(endLabel);
	};

	this.handleEmptyStatement = function (func, statement) { };

	this.handleWhileStatement = function (func, statement) {
		var bodyLabel = func.createLabel();
		var testLabel = func.createLabel();
		func.instructions.push(new Instruction("JMP", testLabel));
		func.placeLabel(bodyLabel);
		this.handle(func, statement.body);
		func.placeLabel(testLabel);
		var comparator = this.emitTest(func, statement.test);
		func.instructions.push(new Instruction(JumpOperator[comparator], bodyLabel));
	};
};

/// Compile and output a complete module.
/// Compilation happens in 3 phases:
///  1. Parse -- translate a JS program to a syntax tree.
///  2. Compile -- first pass compilation of the program to a module.
///  3. resolve -- final stage of linking up unresolved reference in the input program.
function compile(code, output, verbose) {
	var syntax = esprima.parse(code);
	var compiler = new FirstPassCodeGen();
	var module = compiler.compile(syntax);
	module.resolve();
	module.output(output);
	return true;
};

function main() {
	if (process.argv.length != 4) {
		console.error("Usage: node.js compile.js <infile> <outfile>");
		process.exit(1);
	}

	inputPath = process.argv[2];
	outputPath = process.argv[3];

	var code = fs.readFileSync(__dirname + "/b9stdlib.src", 'utf-8');
	code += fs.readFileSync(inputPath, 'utf-8');


	output = fs.openSync(outputPath, "w");
	compile(code, output);
};

main();
