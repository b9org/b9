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
    // codegen.genReturn(false);
    codegen.processDeferred();
    codegen.handleFooters();

});

function FunctionContext(codegen, outer) {
    this.outer = outer;
    this.codegen = codegen;
    this.hasLambda = 0;
    this.usesScope = 0;
    if (outer != undefined) {
        outer.hasLambda = 1;
    }
    this.pushcount = 0;
    this.maxstack = 0;

    this.args = new Object;
    this.args["_hop_"] = this.args.hasOwnProperty;
    this.argcount = 0;
    this.temps = new Object;
    this.temps["_hop_"] = this.temps.hasOwnProperty;
    this.tempcount = 0;

    this.prevInstructionWasReturn = false;
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
            this.codegen.gen("drop", "unused TOS, drop return result to get to expected " + expected);
            this.codegen.gen("");
            this.pushN(-1);
        }
    };

    this.declareGlobal = function (vname) {
        this.codegen.globals[vname] = vname;
    }

    this.declareVariable = function (name) {
        //        if (this.temps[name] != undefined) return; 
        if (this.variableExists(name)) return;

        this.codegen.gen("// Local variable " + name + " offset " + (this.argcount + this.tempcount));
        this.temps[name] = this.argcount + this.tempcount++;
    };

    this.variableExists = function (name) {
        return this.jsLocalGetProperty(this.temps, name) != undefined;
        //return this.temps[name] != undefined;
    };


    this.jsLocalGetProperty = function (o, name) {
        if (o["_hop_"](name)) return o[name];
        return undefined;
    }

    this.variableOffset = function (name) {
        var frame = this;
        var result = new Object();
        result.name = name;
        result.index = 0;
        result.offset = this.jsLocalGetProperty(frame.args, name);
        if (result.offset != undefined) {
            return result;  // this is an arg
        }
        result.offset = this.jsLocalGetProperty(frame.temps, name);
        if (result.offset != undefined) {
            return result;  // this is a temp
        }
        return result;
    };

}

function CodeGen(f) {
    this.main = false;
    this.label = 0;
    this.arrayinit = 0;
    this.nameless = 0;
    this.deferred = [];
    this.filename = f;

    var i, char, id;
    for (i = 0; i < f.length; i++) {
        id = ((id * 32) - id) + f.charCodeAt(i);
        id = id & id;
    }
    if (id < 0) id = 0 - id;
    // 
    id = id & 65535 + (id >> 16);
    this.anonPrefix = "_" + id.toString(26);

    this.globals = new Object;
    this.globals["_hop_"] = this.globals.hasOwnProperty;

    this.currentFunction = new FunctionContext(this, this.currentFunction); // top level function
    this.currentFunction.isTopLevel = true;

    this.currentBreak = function () {
        throw "Invalid Break Statement";
    }
    this.currentContinue = function () {
        throw "Invalid Continue Statement";
    }

    this.gencall = function (name, args, comment) {

        if (name == "_halt_") {
            this.gen("halt");
            return;
        }
        if (name == "") {
            this.gen("calltos " + args, comment);
        } else {
            this.genInstruction("CALL", 1, "TBD: compute offset of: ", name);
        }
    }

    this.genInstruction = function (bc, param, comment) {
        this.gen("createInstruction(" + bc + "," + param + "),", comment);
    }
    this.gen = function (out, comment, tab) {
        if (out == "") {
            console.log("");
            return;
        }
        if (out[0] != '.' && out[0] != '/') {
            this.currentFunction.prevInstructionWasReturn = (out == "return");
        }
        if (tab != undefined) {
            console.log(out);
            return;
        }
        var line = "\t" + out;
        if (comment) {
            var c = 3 - (out.length / 4);
            tabs = "\t// ";
            while (c-- > 0) tabs = "\t" + tabs;
            line = line + tabs + comment;
        }
        console.log(line);
    };

    this.handleHeaders = function () {
        this.gen('#include "b9.h"');
        this.gen("");
    };

    this.handleFooters = function () {
        this.gen('Instruction *b9_exported_functions[] = { program, call_sub};');
        this.gen("");
    };

    this.handle = function (element) {
        if (element == null) {
            this.gen("// Invalid element for code generation");
            return;
        }
        var fname = "handle" + element.type;
        // console.log ("ABOUT TO RUN: " + fname);  
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
            this.gen("jmp " + breakLabel, "break in " + location);
        }
        var saveContinue = this.currentContinue;
        this.currentContinue = function () {
            this.gen("jmp " + continueLabel, "continue in " + location);
        }

        body.call(this);
        this.currentBreak = saveBreak;
        this.currentContinue = saveContinue;
    }

    this.handleSwitchStatement = function (decl) {
        // console.log ("SWITCH " + JSON.stringify (decl)); 

        var endSwitch = "@switch" + this.label;
        this.label++;

        this.flowControlBreakContinue(endSwitch, null, "Switch", function () {
            this.handle(decl.discriminant);
            this.gen("// Switches bodies ");
            this.handleAll(decl.cases);
            this.gen("// End Switch bodies ");
            this.gen(endSwitch + ":");
        });
        this.gen("drop", "end switch ");

    }

    this.handleSequenceExpression = function (decl) {
        console.log("//handleSequenceExpression " + JSON.stringify(decl));
        //        this.handleAll(decl.expressions);
        var expressions = decl.expressions;
        var droplast = !decl.isParameter;
        for (var i = 0; i < expressions.length; i++) {
            var element = expressions[i];
            this.handle(element);
            if (i != (expressions.length - 1) || droplast)
                this.gen("drop");
        };

    }



    this.handleUnaryExpression = function (decl) {
        // console.log ("handleUnaryExpression " + JSON.stringify (decl));  
        if (decl.operator == '-' && decl.argument.type == 'Literal') {
            this.gen("push -" + decl.argument.value, "neg constant -" + decl.argument.value);
            this.currentFunction.pushN(1);
            return;
        }
        if (decl.operator == "typeof") {
            this.handle(decl.argument);
            this.gen("_typeof", "type of tos");
            return;
        }
        if (decl.operator == "+") {
            this.handle(decl.argument);
            return;
        }
        if (decl.operator == "-") {
            this.pushconstant(0);
            this.handle(decl.argument);
            this.gen("sub");
            return;
        }
        if (decl.operator == "~") {
            this.handle(decl.argument);
            this.gencall("_tilde", 1);
            return;
        }

        if (decl.operator == "!") {
            this.handle(decl.argument);
            this.gen("not", "not");
            return "not";
        }
        if (decl.operator == "void") {
            this.handle(decl.argument);
            this.gen("drop");
            this.gen("push undefined ");
            return "void";
        }
        if (decl.operator == "delete") {
            this.gen("// arg to delete" + decl.argument.object);
            if (decl.argument.object) {
                this.handle(decl.argument.object);
            } else {
                this.gen("push null", "global delete");
            }
            console.log("// delete " + JSON.stringify(decl));

            if (decl.argument.type == "Identifier") {
                if (this.currentFunction.variableExists(decl.argument.name)) {
                    this.handle(decl.argument);
                } else {
                    this.pushconstant(decl.argument.name);
                }
            } else {
                if (decl.argument.property.name == undefined) {
                    this.handle(decl.argument.property);
                } else {
                    var name = decl.argument.property.name;
                    var offset = this.currentFunction.variableOffset(name);
                    if (offset.offset != undefined) {
                        this.pushvar(name);
                    } else {
                        this.pushconstant(name);
                    }
                }
            }

            // console.log("// _delete " + JSON.stringify(decl));
            this.gen("_delete", "delete property from object");
            return;
        }

        console.log("// handleUnaryExpression " + JSON.stringify(decl));
        this.gen("halt");
        // halt();
    }

    this.handleWhileStatement = function (decl) {
        //       console.log ("handleWhileStatement " + JSON.stringify (decl));  
        var loopTest = "@loopTest" + this.label;
        var loopEnd = "@loopEnd" + this.label;
        var loopContinue = "@loopContinue" + this.label;
        this.label++;

        this.savehack(function () {
            console.log("// -- while loop test ---");
            this.gen(loopTest + ":");
            var code = this.handle(decl.test);
            var instruction = this.genJmpForCompare(code, "WHILE");
            this.gen(instruction + loopEnd, "genJmpForCompare WHILE " + code); // jump opposite        
        });

        this.flowControlBreakContinue(loopEnd, loopContinue, "WHILE", function () {
            this.savehack(function () {
                //  this.gen(loopBody + ":");
                this.handle(decl.body);
                decl.needResult = false;
                this.gen(loopContinue + ":");
                this.gen("jmp " + loopTest);
                this.gen(loopEnd + ":");
            });
        });
    };



    this.isNumber = function isNumber(num) {
        if (typeof num == 'number') return true;
        return (typeof num == 'string' && !isNaN(parseInt(num)));
    };

    this.hex = function (number, length) {
        var str = number.toString(16).toUpperCase();
        while (str.length < length)
            str = "0" + str;
        return str;
    }

    this.munge = function munge(original) {
        var str = original;
        //   str = str.replace("\n", "\\n");
        // str = str.replace("\r", "\\r");
        //  str = str.replace("\t", "\\t");
        var i;
        var result = "";
        for (i = 0; i < str.length; ++i) {
            if (str.charCodeAt(i) > 127 || str.charCodeAt(i) < 32)
                result += "\\u" + this.hex(str.charCodeAt(i), 4);
            else {
                if (str[i] == '"') {
                    result += '\\"';
                } else {
                    result += str[i];
                }
            }
        }
        return result;
    };

    this.pushconstant = function (constant) {
        if (this.isNumber(constant)) {
            this.genInstruction("PUSH_CONSTANT", constant, " number constant " + (constant - 0));
            return;
        }
        this.currentFunction.pushN(1);
        throw "ONLY HANDLE NUMBERS in B9"
    }

    this.pushvar = function (varname) {
        var offset = this.currentFunction.variableOffset(varname);
        if (offset.offset == varname) {
            this.gen("pushglobal " + varname);
            this.currentFunction.pushN(1);
            return;
        }
        if (offset.offset == undefined) {
            if (varname == "arguments") {
                this.currentFunction.hasLambda = 1; // make sure the method has context to looked at
                this.gen("_arguments", " will enable lambda to ensure context");
                this.currentFunction.pushN(1);
                return;
            }
            if (varname == "this") {
                this.gen("push this ");
            } else {
                if (varname == "undefined") {
                    this.gen("push undefined ");
                } else {
                    this.currentFunction.declareGlobal(varname);
                    this.gen("pushglobal " + varname + " // Y");
                }
            }
        } else {
            this.genInstruction("PUSH_FROM_VAR", offset.offset, "variable " + varname);
        }
        this.currentFunction.pushN(1);
    }

    this.popvar = function (varname) {
        if (varname == "undefined") {
            // nop
            this.gen("drop", "NO ASSIGN TO ANY VARS CALLED UNDEFINED");
            this.currentFunction.pushN(-1);
            return;
        }
        var offset = this.currentFunction.variableOffset(varname);
        if (offset.offset == varname) {
            this.gen("popglobal " + varname);
            this.currentFunction.pushN(-1);
            return;
        }
        if (offset.offset == undefined) {
            this.currentFunction.declareGlobal(varname);
            this.gen("popglobal " + varname);
            this.currentFunction.pushN(-1);
            return;
        }
        if (offset.index == 0) {
            this.genInstruction("POP_INTO_VAR", offset.offset, "variable " + varname);
        } else {
            this.currentFunction.usesScope = 1;
            this.gen("popf " + offset.index + "," + offset.offset, "variable " + varname);
        }
        this.currentFunction.pushN(-1);
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
        if (decl.isTopLevel) {
            this.gen("Instruction " + id + "[] = {", "", false);

            this.gen("decl (" + decl.params.length + ",8),", "(args,temps)  assume max 8 temps");
            this.gen("decl (0,0),", "0: space for JIT address");
            this.gen("decl (0,0),", "1: space for JIT address");
            this.gen("");

        } else {
            this.gen("");
            this.gen(id + ":", "define local label");
            this.gen(".local" + " // lambda now is " + currentFunction.hasLambda);

        }
        this.gen("");
        this.handle(decl.body);

        this.gen("createInstruction(NO_MORE_BYTECODES, 0)};", " end of function");
        this.gen("");

        this.currentFunction = save;
    };

    this.genReturn = function (forced) {
        var currentFunction = this.currentFunction;
        if (!currentFunction.prevInstructionWasReturn || forced) {
            currentFunction.prevInstructionWasReturn = true;
            //this.gen("//.temps " + currentFunction.tempcount);
            //this.gen("//.stack " + currentFunction.maxstack); 
            if (currentFunction.pushcount == 0) {
                this.gen("// free return for undefined");
                this.pushvar("undefined");
            }

            this.genInstruction("RETURN", 0, " forced = " + forced);
        }
    };

    this.processDeferred = function () {
        var todo = this.deferred.shift();;
        while (todo) {
            var id;
            if (todo.id) {
                id = todo.id.name;
            } else {
                id = todo.anonID;
            }
            this.declareFunction(id, todo);
            todo = this.deferred.shift();
        }
    };

    this.handleFunctionDeclaration = function (decl) {
        if (this.currentFunction.isTopLevel) {
            decl.isTopLevel = true;
            this.currentFunction.declareGlobal(decl.id.name);
        } else {
            decl.isTopLevel = false;

            this.gen("// decl var: " + decl.id.name);
            this.currentFunction.declareVariable(decl.id.name);
            this.currentFunction.hasLambda = 1;

            this.gen("push_lambda " + decl.id.name, "has lambda " + this.currentFunction.hasLambda);
            this.currentFunction.pushN(1);
            this.popvar(decl.id.name);
            decl.functionScope = new FunctionContext(this, this.currentFunction);
        }

        this.deferred.push(decl);
        //  this.processDeferred ();
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
                this.pushvar(decl.callee.name);
                this.gencall("", decl.arguments.length);
                this.currentFunction.pushN(0 - decl.arguments.length);
                return;
            }
        }
        //      
        if (decl.callee.type == "ThisExpression") {
            this.pushvar("this");
            this.gen("send " + decl.callee.name + "," + decl.arguments.length);
            this.currentFunction.pushN(0 - decl.arguments.length);
            return;
        }
        if (decl.callee.type == "MemberExpression") {
            this.getProperty(decl.callee.object);
            //   this.gen ("// call expressions" + JSON.stringify (decl)); 
            if (decl.callee.computed) {
                this.pushconstant(decl.callee.property.value);
                this.gen("getproperty_tos ", " get prop from tos for call");
                this.gencall("", decl.arguments.length);
                return;
            }


            this.gen("send " + decl.callee.property.name + "," + decl.arguments.length);
            this.currentFunction.pushN(0 - decl.arguments.length);
            return;
        }

        if (decl.callee.type == "FunctionExpression") {
            this.handle(decl.callee);
            this.gencall("", 0);
            return;
        }
        if (decl.callee.type == "CallExpression") {
            this.handle(decl.callee);
            this.gencall("", 0);
            return;
        }
        if (decl.callee.type == "SequenceExpression") {
            this.handle(decl.callee);
            this.gencall("", 0);
            return;
        }
        this.gen("// unhandled call expressions" + JSON.stringify(decl));
        throw "UNHANDLED CALL EXPRESSION " + decl.callee.type;
    };


    this.handleAll = function (elements) {
        var gen = this;
        elements.forEach(function (element) {
            gen.handle(element);
        });
    };

    this.handleIdentifier = function (decl) {
        // JS(decl);
        this.pushvar(decl.name);
    };

    this.handleArrayExpression = function (decl) {
        //    console.log ("XX" + JSON.stringify (decl));  

        var hack = "array_init" + this.arrayinit;
        this.arrayinit++;

        this.currentFunction.declareVariable(hack);
        this.gen("push_array", " empty array ");
        this.currentFunction.pushN(1);
        this.popvar(hack);
        this.pushvar(hack);

        var gen = this;
        var idx = 0;
        decl.elements.forEach(function (element) {
            if (element == null) {
                gen.gen("//array init slot " + idx + " is undefined");
            } else {
                gen.handle(element);
                gen.pushvar(hack);
                gen.pushconstant(idx);
                gen.gen("setproperty_tos");
                gen.currentFunction.pushN(-3);
            }
            idx++;
        });

        this.gen("push null", "clear the temp hack slot to null ");
        this.currentFunction.pushN(1);
        this.popvar(hack);
        this.arrayinit--;
    };

    this.handleMemberExpression = function (decl) {
        this.getProperty(decl);
    };

    this.updateOpToInstruction = {
        "+=": "add",
        "-=": "sub",
        "/=": "div",
        "*=": "mul",
        ">>>=": "unsignedshiftright",
        ">>=": "bitshiftright",
        "<<=": "bitshiftleft"
    };

    this.handleAssignmentExpression = function (decl) {
        //console.log("// assignment " + JSON.stringify(decl));
        this.gen("");
        if (decl.left.type == "Identifier") {
            var op = this.updateOpToInstruction[decl.operator];
            if (op) {
                this.handle(decl.left); // extra left 
                this.handle(decl.right);
                // this.pushvar(decl.left.name);
                this.gen(op);
                this.currentFunction.pushN(-1);
            } else {
                decl.right.needResult = true;
                this.handle(decl.right);
            }
            if (decl.needResult === true) {
                this.gen("dup ", " extra copy ");
            }
            this.popvar(decl.left.name);
            if (decl.isParameter == true) {
                this.pushvar(decl.left.name);
            }


            return;
        }
        this.handle(decl.right);
        if (decl.left.type == "MemberExpression") {
            if (decl.left.property.name == undefined) {
                this.handle(decl.left.object);
                this.handle(decl.left.property);
                this.gen("setproperty_tos ", " assign 1 get prop from tos (pops 3)");
                this.currentFunction.pushN(-3);
                return;
            } else {
                this.getProperty(decl.left.object);
                if (decl.left.computed) {
                    this.pushvar(decl.left.property.name);
                    this.gen("setproperty_tos ", " assign 2 set prop from tos (pops 3)");
                    this.currentFunction.pushN(-3);
                    return;
                }
                if (decl.left.property.name == "__proto__") {
                    this.gen("_set_prototype", " redirect to primitive-2");
                    this.currentFunction.pushN(-2);
                    return;
                }
                if (decl.left.property.name == "length") {
                    this.gen("_set_length", " redirect to primitive");
                    this.currentFunction.pushN(-1); //pop 2, push 1
                    return;
                }
                this.gen("setproperty " + decl.left.property.name, "setproperty (pop3,push1)");
                this.currentFunction.pushN(-2);
                return;
            }
            XXX();
            this.gen("// XXX");
            this.currentFunction.pushN(-2);
            return;
        }
    };
    this.getProperty = function (decl) {
        // console.log ("// getProperty " + JSON.stringify (decl)); 
        if (decl.type == "MemberExpression") {
            if (decl.property.name == undefined) {
                this.handle(decl.object);
                this.handle(decl.property);
                this.gen("getproperty_tos ", " getproperty_tos pops 2, pushes 1 (1)");
                this.currentFunction.pushN(-1);
            } else {
                this.getProperty(decl.object);
                if (decl.computed) {
                    this.pushvar(decl.property.name);
                    this.gen("getproperty_tos ", " getproperty_tos pops 2, pushes 1 (2)");
                    this.currentFunction.pushN(-1);
                    return;
                }
                if (decl.property.name == "__proto__") {
                    this.gen("_get_prototype", " redirect to primitive");
                    this.currentFunction.pushN(0);
                    return;
                }
                if (decl.property.name == "length") {
                    this.gen("_get_length", " redirect to primitive");
                    this.currentFunction.pushN(0);
                    return;
                }

                this.gen("getproperty " + decl.property.name, " getproperty name  pops 1, pushes 1");
                this.currentFunction.pushN(0);
            }
            this.gen("");
            return;
        }

        this.handle(decl);

        //  console.log ("FAIL " + JSON.stringify (decl)); 
    };

    this.handleVariableDeclaration = function (decl) {
        this.handleAll(decl.declarations);
    };

   

    this.handleThisExpression = function (decl) {
        //     console.log (JSON.stringify (decl));
        this.pushvar("this");
    }

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
            this.gen(loopTest + ":");
            if (decl.test == undefined) {
                //	      this.gen("push true", "No test, loop forever"); 
                code = "nojump";
            } else {
                var code = this.handle(decl.test);
            }
            var instruction = this.genJmpForCompare(code, "FOR");
            this.gen(instruction + loopEnd, "genJmpForCompare FOR " + code); // jump opposite        
        });

        this.flowControlBreakContinue(loopEnd, loopContinue, "FOR",
            function () {
                this.savehack(function () {
                    //  this.gen(loopBody + ":");
                    this.handle(decl.body);

                    this.gen(loopContinue + ":");
                    decl.needResult = false;
                    if (decl.update) decl.update.needResult = false;
                    var expected = this.currentFunction.pushcount;
                    this.handle(decl.update);
                    this.currentFunction.removeUnusedTOS(expected);
                    this.gen("jmp " + loopTest);
                    this.gen(loopEnd + ":");
                });
            });
    };

    this.handleUpdateExpression = function (decl) {
        // console.log (JSON.stringify (decl));  
        if (decl.argument.type == "Identifier") {
            this.pushvar(decl.argument.name);
            this.pushconstant(1);
            if (decl.operator == "++") {
                this.genInstruction("ADD", 0, "// generating var++");
            }
            if (decl.operator == "--") {
                this.genInstruction("SUB", 0, "// generating var--");
            }
            this.popvar(decl.argument.name);
            return;
        }
        throw "Invalid Update Statement support variables only";
    };

    this.genJmpForCompare = function (code, location) {
        if (code == "<") instruction = "JMPLE ";
        return instruction;
    };

    this.handleIfStatement = function (decl) {

        var labelF = "@labelF" + this.label;
        var labelEnd = "@labelEnd" + this.label;
        this.label++;

        this.savehack(function () {
            this.gen("// if statement");
            var code = this.handle(decl.test);
            var instruction = this.genJmpForCompare(code, "IF");
            if (decl.alternate != null) {
                this.gen(instruction + labelF, "genJmpForCompare has false block, IF " + code); // jump to true opposite condition
            } else {
                this.gen(instruction + labelEnd, "genJmpForCompare has no false block, IF " + code); // jump to true opposite condition
            }
            // this.gen("jmp " + labelF); // skip true, go false

        });
        this.savehack(function () { // true part  is fall through from genJmpForCompare 
            this.handle(decl.consequent);
        });
        this.savehack(function () {
            if (decl.alternate != null) {
                // you only have a false if there is code
                // so you only jump if there is code to jump around
                this.gen("jmp " + labelEnd, "SKIP AROUND THE FALSE CODE BLOCK");
                this.gen(labelF + ":");
                this.handle(decl.alternate);
            }
            this.gen(labelEnd + ":");
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
            this.genInstruction("SUB", 0, "");
            this.currentFunction.pushN(-1); // pop 2, push 1
            return decl.operator;
        }
        if (decl.operator == "-") {
            this.genInstruction("ADD", 0, "");
            this.currentFunction.pushN(-1); // pop 2, push 1
            return decl.operator;
        }
        this.gen("halt", "ERROR due to bad operator " + decl.operator);
    };

}

