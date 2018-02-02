/*************************************
CONVERT B9 PORCELAIN TO BINARY MODULE 
**************************************/


var esprima = require('esprima');
var fs = require('fs');
var files = [];
var cmdLineArgs = process.argv.splice(2);


// Push all files passed via the commmand line to the files array
cmdLineArgs.forEach(function(arg) {
    files.push(arg);
});


// Parse each file and generate code
files.forEach(function(filename) {
  //console.log('Reading ' + filename);
  
  // set code to the JS primitive wrappers + JS source file
  var code = fs.readFileSync(__dirname + "/b9stdlib.src", 'utf-8');
  code += fs.readFileSync(filename, 'utf-8');
 
  // parse JS code to AST
  var parsed = esprima.parse(code);

  // create Code Generator
  var codegen = new CodeGen(filename);
 
  handleHeader();
 
  // Arianne: you're trying to keep instructions function specific
	codegen.handleAll(parsed.body);

  // fruitbat: outputting function count
  var fncCount = codegen.deferred.length;
  outputUInt32(fncCount);

  codegen.processDeferred();
	codegen.handleFooters();
	codegen.outputProgram(); 
});

function outputByte(value, file) {
  file = file || "test.txt";
    throw "Error: Value exceeds 0xFF";

  console.log("****" + value);
  fs.appendFile(file, String.fromCharCode(value), (err) => {
    if (err) throw err;
  });
}

function outputUInt32(value, file) {
  file = file || "test.txt";
  const buf = Buffer.allocUnsafe(4);
  buf.writeUInt32LE(value, 0);
  fs.appendFileSync(file, buf);
}

function outputString(string) {
  // Outputting length of string
  var length = string.length;
  outputUInt32(length);
  
  // Outputting raw string
  fs.appendFileSync('test.txt', string, (err) => {
    if (err) throw err;
  });
}

function outputBytecode(bc, param, file) {
}

// robert = fs.openSync("roberty", "w");
// outputBytecode(0x0d, 255, robert);
// fs.writeSync(robert, buf);

function padWithZeros() {
  fs.appendFile('test.txt', String.fromCharCode(0), (err) => {
    if (err) throw err;
  });
}

// Output 'b9module' to binary 
function handleHeader() {
  var header = 'b9module';
  fs.appendFile('test.txt', header, (err) => {
    if (err) throw err;
  });
  //testArray[0] = header;
  //process.stdout.write(header);
  var fncSectionCode = 1;
  outputUInt32(fncSectionCode);
};

function LabelTable() {
  this.labels = [];
  this.make = function() {
    return labels.push(undefined);
  }

  this.place(label, offset) {
    this.labels[label] = offset;
  }

  this.offset(label) {
    return this.labels[label];
  }
}

/// operator - name of operator (NOT CODE)
/// operand - numeric operand, if applicable.
function Instruction(operator, operand) {
  this.operator = operator;
  this.operand = operand;

  this.output = function(file) {
    var encoded = (getOpCode(this.operator) << 24) | (this.operand & 0x00FFFFFF);
    console.log("&&&&" + encoded);
    outputUInt32(encoded, file);
  }
}

function FunctionContext(codegen, outer) {
  
  this.index;
  this.name = "";

	// Add all lines to be output at end of compilation
	this.outputLines = [];

  this.instructions = []; 
  this.testArray = [];

  // Unused - support for nesting functions
  this.outer = outer;
  this.codegen = codegen;
  
  // ???
  this.usesScope = 0;

  // Number of elements currently on the stack
  this.pushcount = 0;
  
  // Max number of elements allowed on the stack
  this.maxstack = 0;

  // args is a new obj with no properties
  this.args = {};
  this.argcount = 0;
  
  // temps stores fnc arguments and temporaries 
  this.temps = {};
  this.tempcount = 0;
 
  this.labels = new LabelTable();
  this.instructions = [];

  this.output(file) {
    outputString(this.name, file);
    outputUInt32(this.index, file);
    outputUInt32(this.arg.length, file);
    outputUInt32(this.temps.length, file);
    for (i in instructions) {
      i.output(file);
    }
  }

  this.updateStackCount = function(count) {
    this.pushcount += count;
    // if number of items will exceed max, increment max
    if (this.pushcount > this.maxstack) {
      this.maxstack = this.pushcount;
    }
  };

  // Drop unused top of stack
  this.dropTOS = function(expected) {
    if (this.pushcount > expected) {
      this.codegen.outputInstruction("DROP", 0, "// drop TOS holding unused return value"); 
      this.updateStackCount(-1);
    }
  };
  
  this.declareVar = function(name) {
    if (this.variableExists(name)) {
      return;
    }
    // map function name to nex available space in temps array, increment tempcount
    this.temps[name] = this.argcount + this.tempcount++;
  };

  this.declareGlobalVar = function(name) {
    this.codegen.globals[name] = name;
  };

  this.variableExists = function(name) {
    return this.temps[name] != undefined;
  };

  // Get variable offset from args or temps
  this.variableOffset = function(name) {
    var frame = this;
    var result = new Object();
    result.name = name;
    result.index = 0; // fruitbat: get rid of index notion and return fail if name not found
    // check for variable in args array
    result.offset = frame.args[name];
    if (result.offset != undefined) {
      return result;
    }
    // check for variable in temps array
    result.offset = frame.temps[name];
    if (result.offset != undefined) {
      return result;
    }
    return result;
  };
}

function StringTable() {
  this.lookup = function(string) {
    var id = this.table.get(string);
    if (id != undefined) {
      return id;
    }

    id = nextId;
    nextId += 1;
    this.table[string] = id;
    return id;
  }

  this.nextId = 0;
  this.table = {};
}

// fruitbat: change this name to getOpcode or something similar 
function getOpCode(bc) {
  switch (bc) {
    case "END_SECTION":
      return 0;
    case "FUNCTION_CALL":
      return 1;
    case "FUNCTION_RETURN":
      return 2;
    case "PRIMITIVE_CALL":
      return 3;
    case "JMP":
      return 4;
    case "DUPLICATE":
      return 5;
    case "DROP":
      return 6;
    case "PUSH_FROM_VAR":
      return 7;
    case "POP_INTO_VAR":
      return 8;
    case "INT_ADD":
      return 9;
    case "INT_SUB":
      return 10;
    case "INT_PUSH_CONSTANT":
      return 13;
    case "INT_NOT":
      return 14;
    case "INT_JMP_EQ":
      return 15;
    case "INT_JMP_GT":
      return 16;
    case "INT_JMP_GE":
      return 17;
    case "INT_JMP_LT":
      return 18;	
    case "INT_JMP_LE":
      return 19;
    case "STR_PUSH_CONSTANT":
      return 20;
    case "STR_JMP_EQ":
      return 21;
    case "STR_JMP_NEQ":
      return 22;
    default:
      return -1;
  }
};

function primitiveCode(primitiveName) {
  var primitives = { "print_string": 0, "print_number": 1 };
  return primitives[primitiveName];
}

function CodeGen(f) {

  // Name of file JS to compile
  this.filename = f;

  // Functions array
  this.functions = [];
  
  this.stringTable = new StringTable();

	// Global variables dictionary
	this.globals = Object.create(null);
 
	// fruitbat: ???
	this.currentFunction = new FunctionContext(this, this.currentFunction); // top level function
	this.currentFunction.isTopLevel = true;


	/* SECTION HANDLERS */

	this.handleAll = function(elements) {
		var codegen = this;
    elements.forEach(function(element) {
		  codegen.handle(element);
	  });
  };

	// Generic handle function to construct appropriate function call
	this.handle = function(element) {
	  if (element == null) {
			throw "// Invalid element for code generation";
		}
		
		// Construct function name based on element type
		var fname = "handle" + element.type;
		var f = this[fname];
		
		// Call the appropriate function
		if (f) {
			return f.call(this, element);
		}
    console.log("Element type: " + element.type);
		// Unsupported JS feature (no code handler)
		throw "Error - No Handler For Type: " + element.type;
	};

  this.outputFunctionSection = function(file) {
       
		const functions = this.functions;
    var fncSectionCode = 1;
    var fncCount = functions.length;
    outputUInt32(fncSectionCode, file);
    outputUInt32(fncCount, file);

    for (f in this.functions) {
      f.output(file);
    }
  }

	this.outputStringSection = function(file) {
		var out = this.strings;

    // Arianne: Handling string section in test.txt
    var stringSectionCode = 2;
    var stringCount = Object.keys(out).length;
    outputUInt32(stringSectionCode);
    outputUInt32(stringCount);
    // Arianne: Outputting the string table
		for (key in out) {
			// Arianne: These have null terminators still
      outputString(key);
      this.outputRawString(key);
		}

	};

	this.outputProgram = function(file) {
    this.outputFunctionSection(file);
    this.outputStringSection(file);
  };

  
  /* STACK OPERATIONS */
  
  this.pushconstant = function(func, constant) {
		if (this.isNumber(constant)) {
			func.instructions.push(new Instruction("INT_PUSH_CONSTANT", constant));
		}
		else if (this.isString(constant)) {
			func.instructions.push(new Instruction(("STR_PUSH_CONSTANT", this.stringTable.lookup(constant))));
			return;
		}
    func.updateStackCount(1);
		throw "ONLY HANDLE NUMBERS in B9";
	}    

	this.pushvar = function(varname) {
		var offset = this.currentFunction.variableOffset(varname);
		if (offset.offset == undefined) {
			throw "Invalid Variable Name " + varname;
		} else {
			this.outputInstruction("PUSH_FROM_VAR", offset.offset, "variable " + varname);
		}
		this.currentFunction.updateStackCount(1);
	}

	this.popvar = function(varname) {
		var offset = this.currentFunction.variableOffset(varname);
		if (offset.index == 0) {
			this.outputInstruction("POP_INTO_VAR", offset.offset, "variable " + varname);
		} else {
			throw "Invalid Variable Name";
		}
		this.currentFunction.updateStackCount(-1);
	}


  /* HANDLE OUTPUT */
  
  this.outputInstruction = function(bc, param, comment) {
    var instruction = new Object();
    instruction.bc = bc; // used to check prev-bytecodename
    instruction.instructionIndex = this.instructionIndex++; // used to compute branch distance
    instruction.stack = this.currentFunction.pushcount; // keep track of stack depth in function line by line
    var gen = this;
    instruction.computeText = function() {
      var computedParm = param;
      if (typeof(param) == "function") {
        computedParm = param(instruction);
      }
      //var out = " I:" + instruction.instructionIndex + " S:" + instruction.stack + " " + comment;
    
			var hexbc = gen.getHexInstruction(bc);  
			if (hexbc == -1) {
				console.log("Invalid Bytecode: " + bc);
			}
			return hexbc.toString(16) + " " + computedParm.toString(16);
    };

		this.prevInstruction = instruction;
		this.outputLines.push(instruction);
    this.currentFunction.instructions.push({"bc" : bc, "param" : param, "index" : instruction.instructionIndex});
    console.log("Bytecode: " + bc + "  Param: " + param + "  Instruction Index: " + instruction.instructionIndex);
  }
  
	this.outputRawString = function(s) {
		this.outputLines.push({ "text": s });
	} 


	/* HANDLE SUPPORTED AST NODES */

	this.handleIdentifier = function(decl) {
  	this.pushvar(decl.name);
  };

	this.handleFunctionDeclaration = function(decl) {
		// TODO handle non top-level functions
		// Note that forward declaration of functions does not specify the
		// number of formal arguemnts the function takes.
		if (this.currentFunction.isTopLevel) {
			decl.isTopLevel = true;
			this.currentFunction.declareGlobalVar(decl.id.name);
		} else {
			throw "Only supports top level functions for now";
		}
		this.deferred.push(decl);	
	};

	this.handleAssignmentExpression = function(decl) {
		if (decl.left.type == "Identifier") {
			var op = this.updateOpToInstruction[decl.operator];
			if (op) {
				this.handle(decl.left); // extra left
				this.handle(decl.right);
				this.outputInstruction(op, 0, "// +=, -= etc");
				this.currentFunction.updateStackCount(-1);
			} else {
				decl.right.needResult = true;
				this.handle(decl.right);
			}
			if (decl.needResult === true) {
				this.outputInstruction("DUPLICATE", 0, "// to be implemented as example in class");
			}
			this.popvar(decl.left.name);
			if (decl.isParameter == true) {
				this.pushvar(decl.left.name);
			}
			return;
		}
		this.handle(decl.right);
	};

	this.handleVariableDeclaration = function(decl) {
		this.handleAll(decl.declarations);
	}

	this.handleVariableDeclarator = function(decl) {
		if (this.currentFunction.isTopLevel) {
			this.currentFunction.declareGlobalVar(decl.id.name);
		} else {
			this.currentFunction.declareVar(decl.id.name);
		}

		if (decl.init) {
			this.handle(decl.init);
			this.popvar(decl.id.name);
		}
	};

	this.handleExpressionStatement = function(decl) {
		var expected = this.currentFunction.pushcount;
		this.handle(decl.expression);
		this.currentFunction.dropTOS(expected);
	};

  // Iterate expressions array
	this.handleSequenceExpression = function(decl) {
	  var expressions = decl.expressions;
    var droplast = !decl.isParameter;
    for (var i = 0; i < expressions.length; i++) {
      var element = expressions[i];
      this.handle(element);
      if (i != (expressions.length -1) || droplast) {
        this.outputInstruction("DROP", 0, "// This is for -Constant");
      }
	  }
  };

	this.handleUnaryExpression = function(decl) {
    if (decl.operator == "-" && decl.argument.type == "Literal") {
      this.outputInstruction("INT_PUSH_CONSTANT", 0 - decl.argument.value, "// This is for -Constant");
      this.currentFunction.updateStackCount(1);
      return;
    }
    if (decl.operator == "+") {
      this.handle(decl.argument);
      return;
    }
    if (decle.operator == "-") {
      this.pushconstant(0);
      this.handle(decl.argument);
      this.outputInstruction("INT_SUB", 0, "// This is for -Constant");
      return;
    }
    if (decl.operator == "!") {
      this.handle(decl.argument);
      this.outputInstruction("INT_NOT", 0, "// This is for !");
      return;
    }
    throw "halt", "Error - No Handler for Type: " + decl;
	};

	this.handleBinaryExpression = function(decl) {
		this.handle(decl.left);
		this.handle(decl.right);
		if (decl.operator == "-") {
			this.outputInstruction("INT_SUB", 0, "");
			this.currentFunction.updateStackCount(-1); // pop 2, push 1
			return decl.operator;
		}
		if (decl.operator == "+") {
			this.outputInstruction("INT_ADD", 0, "");
			this.currentFunction.updateStackCount(-1); // pop 2, push 1
			return decl.operator;
		}
		if (decl.operator == "*") {
			this.outputInstruction("INT_MUL", 0, "");
			this.currentFunction.updateStackCount(-1); // pop 2, push 1
			return decl.operator;
		}
		if (decl.operator == "/") {
			this.outputInstruction("INT_DIV", 0, "");
			this.currentFunction.updateStackCount(-1); // pop 2, push 1
			return decl.operator;
		}

		var code = this.genJmpForCompare(decl.operator)
		if (code) {
			// this will be handled by the jmp which includes the compare operation (i.e jmple, jmpgt)
			return decl.operator;
		}
		throw "This operator is not being handled" + decl.operator;
	};

	this.handleBlockStatement = function(decl) {
		this.handleAll(decl.body);
	}

	this.handleUpdateExpression = function(decl) {
		// console.log (JSON.stringify (decl));
		if (decl.argument.type == "Identifier") {
				this.pushvar(decl.argument.name);
				this.pushconstant(1);
				if (decl.operator == "++") {
						this.outputInstruction("INT_ADD", 0, "// generating var++");
						this.currentFunction.updateStackCount(-1); // pop 2, push 1
				}
				if (decl.operator == "--") {
						this.outputInstruction("INT_SUB", 0, "// generating var--");
						this.currentFunction.updateStackCount(-1); // pop 2, push 1
				}
				this.popvar(decl.argument.name);
				return;
		}
		throw "Invalid Update Statement support variables only";
	};

	this.handleLiteral = function(decl) {
		this.pushconstant(decl.value);
	}

	this.handleIfStatement = function(decl) {
		var labelF = "@labelF" + this.label;
		var labelEnd = "@labelEnd" + this.label;
		this.label++;

		this.savehack(function() {
			var instruction = null;
			var code = this.handle(decl.test);

			if(decl.test.type == "BinaryExpression") {
				 // Binary expressions compile to specialized JMP operations
				instruction = this.genJmpForCompare(code, "IF");
			} else {
				// Unary expressions always compile to a comparison with false.
				this.outputInstruction("INT_PUSH_CONSTANT", 0);
				instruction = "INT_JMP_EQ";
			}

			if (decl.alternate != null) {
				this.outputInstruction(instruction, this.deltaForLabel(labelF),
						"genJmpForCompare has false block, IF " + code);
			} else {
				this.outputInstruction(instruction, this.deltaForLabel(labelEnd),
						"genJmpForCompare has no false block, IF " + code);
			}
		});
		
    this.savehack(function() { // true part  is fall through from genJmpForCompare
			this.handle(decl.consequent);
		});
		
    this.savehack(function() {
			if (decl.alternate != null) {
				// you only have a false if there is code
				// so you only jump if there is code to jump around
				if (this.prevInstruction.bc != "FUNCTION_RETURN") {
					this.outputInstruction("JMP", this.deltaForLabel(labelEnd), "SKIP AROUND THE FALSE CODE BLOCK");
				}
				this.placeLabel(labelF);
				this.handle(decl.alternate);
			}
			this.placeLabel(labelEnd);
		});
	};

	this.handleEmptyStatement = function(decl) {};

	this.handleWhileStatement = function(decl) {
		var loopTest = "@loopTest" + this.label;
		var loopEnd = "@loopEnd" + this.label;
		var loopContinue = "@loopContinue" + this.label;
		this.label++;

		this.savehack(function() {
			this.placeLabel(loopTest);
			var code = this.handle(decl.test);
			var instruction = this.genJmpForCompare(code, "WHILE");
			this.outputInstruction(instruction,
			  this.deltaForLabel(loopEnd), "genJmpForCompare WHILE " + code);
		});

		this.flowControlBreakContinue(loopEnd, loopContinue, "WHILE", function() { this.savehack(function() {
						this.handle(decl.body);
						decl.needResult = false;
						this.placeLabel(loopContinue);
						this.outputInstruction("JMP", this.deltaForLabel(loopTest), "WHILE ");
						this.placeLabel(loopEnd);
				});
		});
	};

	this.handleForStatement = function(decl) {
		var loopTest = "@loopTest" + this.label;
		var loopEnd = "@loopEnd" + this.label;
		var loopContinue = "@loopContinue" + this.label;
		this.label++;

		this.savehack(function() {
			console.log("// -- init ---");

			if (decl.init) this.handle(decl.init);

				console.log("// -- test ---");

				this.placeLabel(loopTest);
				if (decl.test == undefined) {
					code = "nojump";
				} else {
					var code = this.handle(decl.test);
				}
					var instruction = this.genJmpForCompare(code, "FOR");
					this.outputInstruction(instruction, this.deltaForLabel(loopEnd), "genJmpForCompare FOR " + code);
		});

		this.flowControlBreakContinue(loopEnd, loopContinue, "FOR",
			function() {
			this.savehack(function() {
					//  this.outputRawString(loopBody + ":");
					this.handle(decl.body);
					this.placeLabel(loopContinue);
					decl.needResult = false;
					if (decl.update) decl.update.needResult = false;
					var expected = this.currentFunction.pushcount;
					this.handle(decl.update);
					this.currentFunction.dropTOS(expected);
					this.outputInstruction("JMP", this.deltaForLabel(loopTest), "FOR LOOP");
					this.placeLabel(loopEnd);
				});
			});
	};

	this.handleCallExpression = function(decl) {
		// Set up arguments for call
		decl.arguments.forEach(function(element) {
			element.isParameter = true;
		});
		
		this.handleAll(decl.arguments);

		if (decl.callee.type == "Identifier") {
			var offset = this.currentFunction.variableOffset(decl.callee.name);
			if (offset.offset == undefined || offset.global == 1) {
				this.genCallOrPrimitive(decl.callee.name, decl.arguments);
				this.currentFunction.updateStackCount(1 - decl.arguments.length);
				return;
			} else {
				throw "Only handle named functions";
				return;
			}
		}
		this.outputRawString("// unhandled call expressions" + JSON.stringify(decl));
		throw "UNHANDLED CALL EXPRESSION " + decl.callee.type;
	};

	this.handleReturnStatement = function(decl) {
		if (decl.argument != null) {
				this.handle(decl.argument);
		}
		this.genReturn(false);
	};	


  /* GENERATORS */

	this.genFunctionDefinition = function(id, decl) {
		// save the context of the function
		var save;
		if (decl.functionScope) {
				save = this.currentFunction;
				this.currentFunction = decl.functionScope;
		} else {
				save = this.currentFunction;
				this.currentFunction = new FunctionContext(this, undefined); // this.currentFunction);
		}
		currentFunction = this.currentFunction;

		this.instructionIndex = 0;

		functionDecl = this.getFunctionDeclaration(id);
		functionDecl.nargs = decl.params.length;

		// calculate the number of parameters
		var index = 0;
		decl.params.forEach(function(element) {
				currentFunction.args[element.name] = index;
				index++;
		});
		currentFunction.argcount = index;
    
    // Output sizeof function name + function name
		outputString(id);
    outputUInt32(this.functionIndex++);
    
    // Output nargs
    outputUInt32(currentFunction.argcount);

    // Outputting nregs
    outputUInt32(this.currentFunction.tempcount);
	  console.log("***Value of nregs: " + this.currentFunction.tempcount + "***");
 
    var declArgsAndTemps = new Object();
		var func = this.currentFunction;

		this.handle(decl.body);
    
    // Output Instructions
    for (var i = 0; i < currentFunction.instructions.length; i++) {
      var bc = currentFunction.instructions[i].bc;
      var param = currentFunction.instructions[i].param;
      //console.log("Bytecode: " + bc + "  Param: " + param + "  Instruction Index: " + this.instructionIndex);
      var hexbc = this.getHexInstruction(bc);
      outputBytecode(hexbc, param);

      // Arianne - writing all of these to an array to save 

      //codeBoi[i].bc = hexbc;
      //codeBoi[i].param = param;
    }
      
		this.genReturn(true);

		this.genEndOfByteCodes();

    // fruitbat: this isn't doing anything
		//this.outputRawString("");

		functionDecl.nregs = this.currentFunction.tempcount;

		this.currentFunction = save;

    return this.currentFunction;
	};

  this.genCall = function(name, args, comment) {
    this.outputInstruction("FUNCTION_CALL", this.getFunctionIndex(name), "Calling: " + name);
  }

	this.genCallOrPrimitive = function(name, args, comment) {
		if (name == "") {
			throw "Invalid Call with no name to call ";
		}

		if (name == "b9_primitive") {
			this.genPrimitive(args[0], args.slice(1));
		}
		else {
			this.genCall(name, args.length, comment);
		}
	};

	this.genPrimitive = function(name, args, comment) {
    var index = this.primitives.indexOf(name.value);
		this.outputInstruction("PRIMITIVE_CALL", index, 'Calling native: ' + name.value);
		return true;
	};

	this.genJmpForCompare = function(code) {
		if (code == "==") instruction = "INT_JMP_NEQ";
		else if (code == "!=") instruction = "INT_JMP_EQ";
		else if (code == "<=") instruction = "INT_JMP_GT";
		else if (code == "<") instruction = "INT_JMP_GE";
		else if (code == ">") instruction = "INT_JMP_LE";
		else if (code == ">=") instruction = "INT_JMP_LT";
		else throw "Unhandled code";
		return instruction;
	};

	this.genReturn = function(forced) {
		if (this.prevInstruction.bc == "FUNCTION_RETURN") {
			return;
		}
		if (this.currentFunction.pushcount == 0) {
			this.outputInstruction("INT_PUSH_CONSTANT", 0, " Generate Free Return");
		}
		this.outputInstruction("FUNCTION_RETURN", 0, " forced = " + forced);
	}

	this.genEndOfByteCodes = function() {
		this.outputInstruction("END_SECTION", 0, "");		
	};

	this.declareFunction = function(id, decl) {
		var newFunction = {index: this.nextFunctionIndex++, name: id, nargs: -1, nregs: -1};
		this.functions[id] = newFunction;
	};


	/* HANDLE JUMPS AND LABELS */

  this.makeLabel = function() {
    return this.labels.push(undefined);
  }

	this.placeLabel = function(label, index) {
		this.labels[label] = index;
	}

	this.deltaForLabel = function(labelName) {
		var gen = this;
		return function(fromInstruction) {
			// delay computing offset until forward labels are found
			// output is done at the end, all labels will be defined
			var fromIndex = fromInstruction.instructionIndex;
			var toIndex = gen.labels[labelName];
			return toIndex - fromIndex - 1;
		}
	};

	this.flowControlBreakContinue = function(breakLabel, continueLabel, location, body) {
		var saveBreak = this.currentBreak;
		this.currentBreak = function() {
			this.outputInstruction("JMP", this.deltaForLabel(breakLabel), "break in " + location);
		}
		var saveContinue = this.currentContinue;
		this.currentContinue = function() {
			this.outputInstruction("JMP", this.deltaForLabel(continueLabel), "continue in " + location);
		}

		body.call(this);
		this.currentBreak = saveBreak;
		this.currentContinue = saveContinue;
	};


	/* OTHER */

	this.savehack = function(f) {
		var savehack = this.currentFunction.pushcount;
		f.call(this);
		this.currentFunction.pushcount = savehack;
	} 

	this.getFunctionDeclaration = function (id) {
		if (this.functions[id] == undefined) {
				this.declareFunction(id);
		}
		return this.functions[id];
	};
	
  // Returns the function index for CALL If the function has not been
  // declared yet, it creates a stub declaration for the function.
  this.getFunctionIndex = function(id) {
    return this.getFunctionDeclaration(id).index;
  };

	this.isNumber = function isNumber(num) {
		return typeof num == "number";
	};	

	this.isString = function isString(num) {
		return typeof num == 'string';
	};

	this.getStringIndex = function(id) {
		if (this.strings[id] != undefined) {
			return this.strings[id];
		} else {
			this.strings[id] = this.nextStringIndex++;
			return this.strings[id];
		}
	};

}
