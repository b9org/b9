// convert subset of JavaScript to B9

var fs = require('fs');
var esprima = require('esprima');
var args = process.argv.splice(2);
var files = [];

args.forEach(function (arg) {
    files.push(arg);
});

files.forEach(function (filename) {
    var content = fs.readFileSync(filename, 'utf-8');
    var parsed = esprima.parse(content);
    var codegen = new CodeGen(filename);
    codegen.handleHeaders();
    codegen.handleAll(parsed.body);
    codegen.processDeferred();
    codegen.handleFooters();

    codegen.outputProgram();

});

function FunctionContext(codegen, outer) {
    this.outer = outer;
    this.codegen = codegen;
    this.usesScope = 0;
    this.pushcount = 0;
    this.maxstack = 0;

    this.args = Object.create(null);
    this.argcount = 0;
    this.temps = Object.create(null);
    this.tempcount = 0;
    this.isTopLevel = false;

    this.getsavedcontext = function () {
        return this.outer;
    };
    this.pushN = function (count) {
        this.pushcount += count;
        if (this.pushcount > this.maxstack) {
            this.maxstack = this.pushcount;
        }
    }
    this.removeUnusedTOS = function (expected) {
        if (this.pushcount > expected) {
            this.codegen.outputInstruction("DROP", 0, "// unused TOS, drop return result to get to expected ");
            this.pushN(-1);
        }
    };

    this.declareGlobal = function (vname) {
        this.codegen.globals[vname] = vname;
    }
    this.declareVariable = function (name) {
        if (this.variableExists(name)) return;
        this.temps[name] = this.argcount + this.tempcount++;
    };
    this.variableExists = function (name) {
        return this.temps[name] != undefined;
    };
    this.variableOffset = function (name) {
        var frame = this;
        var result = new Object();
        result.name = name;
        result.index = 0;
        result.offset = frame.args[name];
        if (result.offset != undefined) {
            return result;  // this is an arg
        }
        result.offset = frame.temps[name];
        if (result.offset != undefined) {
            return result;  // this is a temp
        }
        return result;
    };
}

function CodeGen(f) {

    this.filename = f;
    // number labels globally 
    this.label = 0;
    // functions get compiled all at the end, so all global definitions can be known
    this.deferred = [];

    // all the functions you compile in this file. 
    this.functions = {}
    this.nextFunctionIndex = 0;

    this.labels = new Object();
    this.instructionIndex = 0;
    this.outputLines = [];

    this.globals = Object.create(null);
    this.currentFunction = new FunctionContext(this, this.currentFunction); // top level function
    this.currentFunction.isTopLevel = true;

    this.outputProgram = function () {
        for (var i = 0; i < this.outputLines.length; i++) {
            if (this.outputLines[i].computeText) {
                console.log(this.outputLines[i].computeText());
            } else {
                console.log(this.outputLines[i].text);
            }
        }
    }

    this.currentBreak = function () {
        throw "No Break Destination Set, Invalid Break Statement";
    }
    this.currentContinue = function () {
        throw "No Continue Destination Set, Invalid Break Statement";
    }

    this.gencall = function (name, args, comment) {
        if (name == "") {
            throw "Invalid Call with no name to call ";
        } else {
            this.outputInstruction("CALL", this.getFunctionIndex(name), "Offset of: " + name);
        }
    }
    this.outputRawString = function (s) {
        this.outputLines.push({ "text": s });
    }
    this.outputComment = function (comment) {
        this.outputRawString("// " + comment);
    }
    this.placeLabel = function (labelName) {
        this.labels[labelName] = this.instructionIndex;
    }
    this.deltaForLabel = function (labelName) {
        var gen = this;
        return function (fromInstruction) {
            // delay computing offset until forward labels are found
            // output is done at the end, all labels will be defined
            var fromIndex = fromInstruction.instructionIndex;
            var toIndex = gen.labels[labelName];
            return toIndex - fromIndex - 1;
        }
    }

    this.outputInstruction = function (bc, param, comment) {
        var instruction = new Object();
        instruction.bc = bc;  // used to check prev-bytecodename
        instruction.instructionIndex = this.instructionIndex++;  // used to compute branch distance 
        instruction.stack = this.currentFunction.pushcount;  // keep track of stack depth in function line by line
        var gen = this;
        instruction.computeText = function () {
            var computedParm = param;
            if (typeof (param) == "function") computedParm = param(instruction);
            var out = " I:" + instruction.instructionIndex + " S:" + instruction.stack + " " + comment;
            return gen.outputInstructionText(
                "createInstruction(" + bc + "," + computedParm + "),",
                out,
                true);
        };
        this.prevInstruction = instruction;
        this.outputLines.push(instruction);
    }

    this.outputInstructionText = function (out, comment, tab) {
        while (out.length < 32) out += " ";
        var line = "\t" + out;
        if (comment) {
            line = line + "\t// " + comment;
        }
        return line;
    };

    this.handleHeaders = function () {
        this.outputRawString('#include "b9.h"');
    };

    this.handleFooters = function () {
        var out = this.functions;

        this.outputRawString('struct ExportedFunctionData  b9_exported_functions[] = {');
        for (key in out) {
            var init = '    {' + '"' + key + '",' +  key + ',0},';
            this.outputRawString(init, '// ' + this.functions[key]);
        }
        this.outputRawString('    {NO_MORE_FUNCTIONS,0,0}');
        this.outputRawString('};');
    };

    this.handle = function (element) {
        if (element == null) {
            throw "// Invalid element for code generation";
        }
        var fname = "handle" + element.type;
        var f = this[fname];
        if (f) {
            return f.call(this, element);
        }
        // If you use some JS feature with no code generator, error with no handler
        throw "halt", "Error - No Handler For Type: " + element.type;
    };

    this.flowControlBreakContinue = function (breakLabel, continueLabel, location, body) {
        var saveBreak = this.currentBreak;
        this.currentBreak = function () {
            this.outputInstruction("JMP", this.deltaForLabel(breakLabel), "break in " + location);
        }
        var saveContinue = this.currentContinue;
        this.currentContinue = function () {
            this.outputInstruction("JMP", this.deltaForLabel(continueLabel), "continue in " + location);
        }

        body.call(this);
        this.currentBreak = saveBreak;
        this.currentContinue = saveContinue;
    }

    this.handleSequenceExpression = function (decl) {
        //console.log("//handleSequenceExpression " + JSON.stringify(decl)); 
        var expressions = decl.expressions;
        var droplast = !decl.isParameter;
        for (var i = 0; i < expressions.length; i++) {
            var element = expressions[i];
            this.handle(element);
            if (i != (expressions.length - 1) || droplast)
                this.outputInstruction("DROP", 0, "// This is for -Constant");
        };

    }

    this.handleUnaryExpression = function (decl) {
        // console.log ("handleUnaryExpression " + JSON.stringify (decl));  
        if (decl.operator == '-' && decl.argument.type == 'Literal') {
            this.outputInstruction("PUSH_CONSTANT", 0 - decl.argument.value, "// This is for -Constant");
            this.currentFunction.pushN(1);
            return;
        }
        if (decl.operator == "+") {
            this.handle(decl.argument);
            return;
        }
        if (decl.operator == "-") {
            this.pushconstant(0);
            this.handle(decl.argument);
            this.outputInstruction("SUB", 0, "// This is for -Constant"); ""
            return;
        }
        if (decl.operator == "!") {
            this.handle(decl.argument);
            this.outputInstruction("NOT", 0, "// This is for !");
            return;
        }
        throw "halt", "Error - No Handler For Type: " + decl;
    }

    this.handleWhileStatement = function (decl) {
        //       console.log ("handleWhileStatement " + JSON.stringify (decl));  
        var loopTest = "@loopTest" + this.label;
        var loopEnd = "@loopEnd" + this.label;
        var loopContinue = "@loopContinue" + this.label;
        this.label++;

        this.savehack(function () {
            this.placeLabel(loopTest);
            var code = this.handle(decl.test);
            var instruction = this.genJmpForCompare(code, "WHILE");
            this.outputInstruction(instruction,
                this.deltaForLabel(loopEnd), "genJmpForCompare WHILE " + code);
        });

        this.flowControlBreakContinue(loopEnd, loopContinue, "WHILE", function () {
            this.savehack(function () {
                this.handle(decl.body);
                decl.needResult = false;
                this.placeLabel(loopContinue);
                this.outputInstruction("JMP", this.deltaForLabel(loopTest), "WHILE ");
                this.placeLabel(loopEnd);
            });
        });
    };

    this.isNumber = function isNumber(num) {
        if (typeof num == 'number') return true;
        return (typeof num == 'string' && !isNaN(parseInt(num)));
    };

    this.pushconstant = function (constant) {
        if (this.isNumber(constant)) {
            this.outputInstruction("PUSH_CONSTANT", constant, " number constant " + (constant - 0));
            this.currentFunction.pushN(1);
            return;
        }
        this.currentFunction.pushN(1);
        throw "ONLY HANDLE NUMBERS in B9"
    }

    this.pushvar = function (varname) {
        var offset = this.currentFunction.variableOffset(varname);
        if (offset.offset == undefined) {
            throw "Invalid Variable Name";
        } else {
            this.outputInstruction("PUSH_FROM_VAR", offset.offset, "variable " + varname);
        }
        this.currentFunction.pushN(1);
    }

    this.popvar = function (varname) {
        var offset = this.currentFunction.variableOffset(varname);
        if (offset.index == 0) {
            this.outputInstruction("POP_INTO_VAR", offset.offset, "variable " + varname);
        } else {
            throw "Invalid Variable Name";
        }
        this.currentFunction.pushN(-1);
    }

    // returns the function index for CALL
    this.getFunctionIndex = function (id) {
        if (this.functions[id] != undefined) {
            return this.functions[id];
        } else {
            this.functions[id] = this.nextFunctionIndex++;
            return this.functions[id];
        }
    }

    this.declareFunction = function (id, decl) {
        var save;

        if (decl.functionScope) {
            save = this.currentFunction;
            this.currentFunction = decl.functionScope;
        } else {
            save = this.currentFunction;
            this.currentFunction = new FunctionContext(this, undefined); // this.currentFunction); 
        }

        currentFunction = this.currentFunction;
        var index = 0;
        decl.params.forEach(function (element) {
            currentFunction.args[element.name] = index;
            index++;
        });
        currentFunction.argcount = index;

        /* This will make sure the current function has a valid index */
        this.getFunctionIndex(id);

        if (decl.isTopLevel) {
            this.instructionIndex = 0;
            this.outputRawString("Instruction " + id + "[] = {", "", false);

            var declArgsAndTemps = new Object();
            var fThis = this;
            var func = fThis.currentFunction;
            declArgsAndTemps.computeText = function () {
                return fThis.outputInstructionText(
                    "decl (" + decl.params.length + "," + func.tempcount + "),",
                    "args:" + decl.params.length + " temps:" + func.tempcount,
                    true);
            };
            this.outputLines.push(declArgsAndTemps);

            // this.outputRawString(this.outputInstructionText(
            //     "decl (" + decl.params.length + ",8),",
            //     "args and temps",
            //     true));

            this.outputRawString(this.outputInstructionText(
                "decl (0,0),",
                "0: space for JIT address",
                true));
            this.outputRawString(this.outputInstructionText(
                "decl (0,0),",
                "0: space for JIT address",
                true));

        } else {
            throw "Only supports top level functions for now";
        }
        this.outputRawString("");
        this.handle(decl.body);
        this.genReturn(true);
        this.outputRawString("createInstruction(NO_MORE_BYTECODES, 0)};", " end of function");
        this.outputRawString("");

        this.currentFunction = save;
    };

    this.genReturn = function (forced) {
        if (this.prevInstruction.bc == "RETURN") {
            return;
        }
        if (this.currentFunction.pushcount == 0) {
            this.outputInstruction("PUSH_CONSTANT", 0, " Generate Free Return");
        }
        this.outputInstruction("RETURN", 0, " forced = " + forced);
    };

    this.processDeferred = function () {
        var todo = this.deferred.shift();;
        while (todo) {
            this.declareFunction(todo.id.name, todo);
            todo = this.deferred.shift();
        }
    };

    this.handleFunctionDeclaration = function (decl) {
        if (this.currentFunction.isTopLevel) {
            decl.isTopLevel = true;
            this.currentFunction.declareGlobal(decl.id.name);
        } else {
            throw "Only supports top level functions for now";
        }
        this.deferred.push(decl);
    };

    this.handleExpressionStatement = function (decl) {
        //  this.gen ("// expression statementl" + JSON.stringify (decl));
        var expected = this.currentFunction.pushcount;
        this.handle(decl.expression);
        this.currentFunction.removeUnusedTOS(expected);
    };

    this.handleCallExpression = function (decl) {
        var gen = this;
        decl.arguments.forEach(function (element) {
            element.isParameter = true;
        });

        this.handleAll(decl.arguments);
        if (decl.callee.type == "Identifier") {
            var offset = this.currentFunction.variableOffset(decl.callee.name);
            if (offset.offset == undefined || offset.global == 1) {
                this.gencall(decl.callee.name, decl.arguments.length);
                this.currentFunction.pushN(1 - decl.arguments.length);
                return;
            } else {
                throw "Only handle named functions";
                return;
            }
        }
        this.outputRawString("// unhandled call expressions" + JSON.stringify(decl));
        throw "UNHANDLED CALL EXPRESSION " + decl.callee.type;
    };


    this.handleAll = function (elements) {
        var gen = this;
        elements.forEach(function (element) {
            gen.handle(element);
        });
    };

    this.handleIdentifier = function (decl) {
        this.pushvar(decl.name);
    };

    this.updateOpToInstruction = {
        "+=": "ADD",
        "-=": "SUB",
        "/=": "DIV",
        "*=": "MUL"
    };

    this.handleAssignmentExpression = function (decl) {
        //console.log("// assignment " + JSON.stringify(decl));

        if (decl.left.type == "Identifier") {
            var op = this.updateOpToInstruction[decl.operator];
            if (op) {
                this.handle(decl.left); // extra left 
                this.handle(decl.right);
                this.outputInstruction(op, 0, "// +=, -= etc");
                this.currentFunction.pushN(-1);
            } else {
                decl.right.needResult = true;
                this.handle(decl.right);
            }
            if (decl.needResult === true) {
                this.outputInstruction("DUP", 0, "// to be implemented as example in class");
            }
            this.popvar(decl.left.name);
            if (decl.isParameter == true) {
                this.pushvar(decl.left.name);
            }
            return;
        }
        this.handle(decl.right);
    };

    this.handleVariableDeclaration = function (decl) {
        this.handleAll(decl.declarations);
    };

    this.handleVariableDeclarator = function (decl) {
        //     console.log (JSON.stringify (decl));
        if (this.currentFunction.isTopLevel) {
            this.currentFunction.declareGlobal(decl.id.name);
        } else {
            this.currentFunction.declareVariable(decl.id.name);
        }

        if (decl.init) {
            this.handle(decl.init);
            this.popvar(decl.id.name);
        }
    };

    this.handleBlockStatement = function (decl) {
        // console.log ("HANDLE BLOCK " + JSON.stringify (decl));
        this.handleAll(decl.body);
    };

    this.savehack = function (f) {
        var savehack = this.currentFunction.pushcount;
        f.call(this);
        this.currentFunction.pushcount = savehack;
    }


    this.handleEmptyStatement = function (decl) {
    }

    this.handleForStatement = function (decl) {
        //  console.log (JSON.stringify (decl));
        var loopTest = "@loopTest" + this.label;
        //   var loopBody = "@loopBody" + this.label;
        var loopEnd = "@loopEnd" + this.label;
        var loopContinue = "@loopContinue" + this.label;
        this.label++;

        this.savehack(function () {
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
            function () {
                this.savehack(function () {
                    //  this.outputRawString(loopBody + ":");
                    this.handle(decl.body);
                    this.placeLabel(loopContinue);
                    decl.needResult = false;
                    if (decl.update) decl.update.needResult = false;
                    var expected = this.currentFunction.pushcount;
                    this.handle(decl.update);
                    this.currentFunction.removeUnusedTOS(expected);
                    this.outputInstruction("JMP", this.deltaForLabel(loopTest), "FOR LOOP");
                    this.placeLabel(loopEnd);
                });
            });
    };

    this.handleUpdateExpression = function (decl) {
        // console.log (JSON.stringify (decl));  
        if (decl.argument.type == "Identifier") {
            this.pushvar(decl.argument.name);
            this.pushconstant(1);
            if (decl.operator == "++") {
                this.outputInstruction("ADD", 0, "// generating var++");
                this.currentFunction.pushN(-1); // pop 2, push 1
            }
            if (decl.operator == "--") {
                this.outputInstruction("SUB", 0, "// generating var--");
                this.currentFunction.pushN(-1); // pop 2, push 1
            }
            this.popvar(decl.argument.name);
            return;
        }
        throw "Invalid Update Statement support variables only";
    };

    this.genJmpForCompare = function (code) {
        if (code == "==") instruction = "JMP_NEQ ";
        if (code == "!=") instruction = "JMP_EQ ";
        if (code == "<=") instruction = "JMP_GT ";
        if (code == "<") instruction = "JMP_GE ";
        if (code == ">") instruction = "JMP_LE ";
        if (code == ">=") instruction = "JMP_LT ";
        return instruction;
    };

    this.handleIfStatement = function (decl) {
        var labelF = "@labelF" + this.label;
        var labelEnd = "@labelEnd" + this.label;
        this.label++;

        this.savehack(function () {
            var code = this.handle(decl.test);
            var instruction = this.genJmpForCompare(code, "IF");
            if (decl.alternate != null) {
                this.outputInstruction(instruction, this.deltaForLabel(labelF),
                    "genJmpForCompare has false block, IF " + code);
            } else {
                this.outputInstruction(instruction, this.deltaForLabel(labelEnd),
                    "genJmpForCompare has no false block, IF " + code);
            }
        });
        this.savehack(function () { // true part  is fall through from genJmpForCompare 
            this.handle(decl.consequent);
        });
        this.savehack(function () {
            if (decl.alternate != null) {
                // you only have a false if there is code
                // so you only jump if there is code to jump around 
                if (this.prevInstruction.bc != "RETURN") {
                    this.outputInstruction("JMP", this.deltaForLabel(labelEnd), "SKIP AROUND THE FALSE CODE BLOCK");
                }
                this.placeLabel(labelF);
                this.handle(decl.alternate);
            }
            this.placeLabel(labelEnd);
        });
    };

    this.handleReturnStatement = function (decl) {
        if (decl.argument != null) {
            this.handle(decl.argument);
        }
        this.genReturn(false);
    };

    this.handleLiteral = function (decl) {
        this.pushconstant(decl.value);
    };
    this.handleBinaryExpression = function (decl) {
        this.handle(decl.left);
        this.handle(decl.right);
        if (decl.operator == "-") {
            this.outputInstruction("SUB", 0, "");
            this.currentFunction.pushN(-1); // pop 2, push 1
            return decl.operator;
        }
        if (decl.operator == "+") {
            this.outputInstruction("ADD", 0, "");
            this.currentFunction.pushN(-1); // pop 2, push 1
            return decl.operator;
        }
        if (decl.operator == "*") {
            this.outputInstruction("MUL", 0, "");
            this.currentFunction.pushN(-1); // pop 2, push 1
            return decl.operator;
        }
        if (decl.operator == "/") {
            this.outputInstruction("DIV", 0, "");
            this.currentFunction.pushN(-1); // pop 2, push 1
            return decl.operator;
        }
        var code = this.genJmpForCompare(decl.operator)
        if (code) {
            // this will be handled by the jmp which includes the compare operation (i.e jmple, jmpgt)
            return decl.operator;
        }
        throw "This operator is not being handled" + decl.operator;
    };
}

