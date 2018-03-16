---
layout: devJourney
title: Build your own Language Runtime
---

* Table of Contents
{:toc}

## 0.0 Introduction

Welcome to our tutorial! If you're interested in building your own language runtime, you've come to the right place. Base9 is a miniature language runtime, and thanks to OMR and JitBuilder, it even has a **Just In Time (JIT) Compiler**! The goal of this tutorial is not to teach you how to build Base9, but rather to pack your arsenal full of tools to build your own language runtime. We don't want to bog you down with a bunch of unnecessary information, so we'll keep things very straightfoward, providing useful links/definitions along the way for you to peruse optionally and at your own convenience. If you'd like to get familiar with some of the vocabulary we'll be using, feel free to visit our [dictionary]. Lets get started!

[dictionary]: ./Dictionary.md

## 1.0 Base 9


### 1.1 A Brief Overview of Base 9

If you want to fiddle around with Base9, check out our [set-up page](./SetupBase9.md) for set-up instructions.

Base9 is an educational **language runtime**. It's front-end language, b9porcelain, is a small subset of JavaScript. It compiles into a simple set of **bytecodes** which run on a primitive **interpreter**. We've also plugged in [OMR] and [JitBuilder] to provide our runtime with a **JIT compiler**! 

[OMR]: https://www.eclipse.org/omr/
[JitBuilder]: https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/

Base9 has several major components that we'll discuss throughout the course of this tutorial. We'll try to provide insight about why we made the design decisions that we did, and how we built up the pieces. We encourage you to remember that much of our implementation is made up of design decisions suited to our project. Along the way, you may wish to deviate from these decisions in order to best suit your own project.

{% include image.liquid url="./images/b9overview.png" description="Base9 Overview" %}

The Base9 Overview Diagram depicts the Ahead-of-Time compilation unit and the Virtual Machine unit. The Ahead-of-Time unit runs the b9porcelain source code through our frontend compiler. The frontend compiler outputs a binary module. The binary module is passed to the deserializer, which converts it to a C++ data structure that we've named "Module", and which will henceforth be refered to as our "in memory Module".

The Virtual Machine unit is comprised of the Interpreter and the JIT (not depicted here). The in memory Module is passed to the VM, which runs the bytecodes of the program. 

We'll discuss each of the above components in detail in the upcoming sections. 


### 1.2 B9porcelain, our Frontend Language

b9porcelain is our front-end language. As mentioned, it's a subset of JavaScript. Let's have a look at some code: 
 
 ```js
 function fib(a) {
    if (a < 3) {
        return 1;
    } else {
        return fib(a - 1) + fib(a - 2);
    }
}

function b9main() {
    b9PrintString("");
    b9PrintString("Fibonacci");
    var a = 1;
    while (a <= 20) {
        b9PrintNumber(fib(a));
        a++;
    }
}
```

Above is a classic program that we all know and love, Fibonacci. `b9main()` is using two of the Base9 **primitive functions** (from our primitive function table), `b9PrintString` and `b9PrintNumber`, to print to console. Currently, b9porcelain is only capable of operating on integers. It can output strings, but it can't (yet) perform operations on them.

b9porcelain source code is passed as input into our [frontend compiler], which uses [Esprima] to convert the program into an [Abstract Syntax Tree] (or AST). Our frontend compiler walks the AST and converts it into our portable binary format. The portable binary format is represented as our Binary Module.  

[frontend compiler]: https://github.com/b9org/b9/blob/master/js_compiler/compile.js
[Esprima]: http://esprima.org
[Abstract Syntax Tree]: https://en.wikipedia.org/wiki/Abstract_syntax_tree

For a detailed overview of how we've designed/built our front-end compiler and binary format, visit the link below:

[Frontend Compiler and Binary Format](./FrontendAndBinaryMod.md)


### 1.3 Building the Module

The Base9 [deserializer] is responsible for taking the binary module (which is output by the frontend compiler), and converting it into the in memory Module (which contains the Base9 bytecodes) to be run by the VM. 

[deserializer]: (https://github.com/b9org/b9/blob/master/b9/src/deserialize.cpp)

Click the link below to learn more about our Base9 deserializer and disassembler:

[Base9 deserializer and disassembler](./Deserializer.md)

The in memory Module is represented in Base9 as follows:

```c++
struct Module {
  std::vector<FunctionDef> functions;
  std::vector<std::string> strings;

  std::size_t getFunctionIndex(const std::string& name) const {
    for (std::size_t i = 0; i < functions.size(); i++) {
      if (functions[i].name == name) {
        return i;
      }
    }
    throw FunctionNotFoundException{name};
  }
};
```

The Module contains only 2 fields; a `functions` vector, and a `strings` vector. `functions` contains a set of function definitions, or `FunctionDef`'s. More on that in a moment. The strings vector in the Module contain the contents of the string table, which holds any strings used by the b9porcelain program. Lastly in the Module, we see a function called `getFunctionIndex`, which traverses the functions vector using a function name, and returns that functions position in the vector. 

Now let's take a look at our `FunctionDef` struct: 

```c++
struct FunctionDef {
  // Copy Constructor
  FunctionDef(const std::string& name, std::uint32_t index,
              const std::vector<Instruction>& instructions,
              std::uint32_t nargs = 0, std::uint32_t nregs = 0)
      : name{name},
        index{index},
        instructions{instructions},
        nargs{nargs},
        nregs{nregs} {}

  // Move Constructor
  FunctionDef(const std::string& name, std::uint32_t index,
              std::vector<Instruction>&& instructions, std::uint32_t nargs = 0,
              std::uint32_t nregs = 0)
      : name{name},
        index{index},
        instructions{std::move(instructions)},
        nargs{nargs},
        nregs{nregs} {}

  // Function Data
  std::string name;
  uint32_t index;
  std::uint32_t nargs;
  std::uint32_t nregs;
  std::vector<Instruction> instructions;
};
```

The fields of the `FunctionDef` are the name of the function, the index of the function in the function vector, the number of arguments, the number of registers, and the `Instructions` vector (which contains the bytecodes). 


### 1.4 Execution Model

The Base9 Virtual Machine is a relatively simple C++ class. It loads an in memory module Module which has been compiled from b9porcelain source code, and runs the program. The Module is created by converting b9porcelain into an executable sequence of bytecodes represented in our [binary format]. The binary format is then deserialized to create the in memory Module, which is passed to the VM. The VM then has two ways of handling the Module. It can either execute the Module's bytecodes line by line in the interpreter, or it can choose to JIT compile the bytecodes and run that version instead. 

[binary format]: ./FrontendAndBinaryMod.md

{% include image.liquid url="./images/b9porcelainToBC.png" description="b9porcelain to Bytecodes" %}

The above diagram shows a direct translation between b9porcelain source code and it's corresponding bytecodes.

{% include image.liquid url="./images/vmDesign.png" description="Virtual Machine Design" %}

The above diagram shows our Virtual Machine Design. The VM takes the bytecodes from the Module, and runs them through either the Interpreter or the JIT compiler. The Interpreter will process the bytecodes directly, and the JIT compiler converts them to native machine code. We employ user flags to tell the VM to JIT compile an entire program and to interpret nothing. To run a fully JIT compiled program, navigate to the `build/` directory and run:

`./b9run/b9run -jit ./test/<binary module>.b9mod`


### 1.5 The Virtual Machine

The Base9 VM has been subject to many design decisions. Please note, as with many of our components, there are multiple ways to implement a language runtime. As discussed above, the Base9 virtual machine takes a Module structure as input. Let's take a look at our [main function] to gain a better understanding of how this works. Our main function calls `run(runtime, cfg);`:

[main function]: https://github.com/b9org/b9/blob/master/b9run/main.cpp

```
static void run(OMR::Om::ProcessRuntime& runtime, const RunConfig& cfg) {
  b9::VirtualMachine vm{runtime, cfg.b9};

  std::ifstream file(cfg.moduleName, std::ios_base::in | std::ios_base::binary);
  auto module = b9::deserialize(file);
  vm.load(module);

  if (cfg.b9.jit) {
    vm.generateAllCode();
  }

  size_t functionIndex = module->getFunctionIndex(cfg.mainFunction);
  for (std::size_t i = 0; i < cfg.loopCount; i += 1) {
    auto result = vm.run(functionIndex, cfg.usrArgs);
    std::cout << std::endl << "=> " << result << std::endl;
  }
}
```

One of your first considerations will be how the user interacts with your runtime. In Base9, we compile b9porcelain into a binary module using the following command:

`node ./compile.js <in> <out>`

To run our VM using the binary module, we do the following:

`b9run [<option>...] [--] <module> [<arg>...]`

The `<module>` argument is the binary module, which we created using `compile.js`. If you take a look in [main.cpp], you'll notice our argument parsing function, which collects the `<module>` argument from the command line. If you choose to run your VM from the command line, you will need to do something similar. 

[main.cpp]: https://github.com/b9org/b9/blob/master/b9run/main.cpp

Again, how we structure and handle our binary format was a personal design decision that may or may not work for you. We chose to read our binary file into our in memory Module, and to load it into the VM to run. Lets take a closer look at how the VM runs our Module. 

### 1.6 The Interpreter

Once the VM is instantiated and has loaded the Module, it calls the `VirtualMachine::run` function in [b9/src/core.cpp], which in turn calls the interpreter. The interpreter can be found in [b9/src/ExecutionContext.cpp] in the `interpret` function (`StackElement ExecutionContext::interpret(const std::size_t functionIndex)`). The `interpret` function contains a while loop wrapped around a switch statement. The while loop iterates the bytecodes in a given function and executes the appropriate code for each particular bytecode it encounters. 

[b9/src/core.cpp]: https://github.com/b9org/b9/blob/master/b9/src/core.cpp
[b9/src/ExecutionContext.cpp]: https://github.com/b9org/b9/blob/master/b9/src/ExecutionContext.cpp


### 1.7 Base9 Summary

To conclude this section, let's briefly walk over the components we've covered thus far. `b9porcelain` is our front-end language. It is compiled by our [compiler] using Esprima, and eventually converted into our binary module. The binary module is fed to our [deserializer], which converts it into the Module structure to be consumed by the VM. The VM is itself a C++ class, and can be found in [VirtualMachine.hpp]. Once instantiated, the VM loads and runs the Module through the [interpreter]. 

[compiler]: https://github.com/b9org/b9/blob/master/js_compiler/compile.js
[deserializer]: https://github.com/b9org/b9/blob/master/b9/src/deserialize.cpp
[VirtualMachine.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/VirtualMachine.hpp
[interpreter]: https://github.com/b9org/b9/blob/master/b9/src/ExecutionContext.cpp



## 2.0 Plugging in the JIT Compiler 

As we mentioned earlier, our JIT compiler is made possible by [OMR] and [JitBuilder]. We keep OMR in our [third_party] directory as a **submodule**. JitBuilder exists as part of OMR.

[OMR]: https://www.eclipse.org/omr/
[JitBuilder]: https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/
[third_party]: https://github.com/b9org/b9/tree/master/third_party

With our OMR submodule, and with the correct `#include`'s in our Base9 headers, using OMR and JitBuilder functionality is easy!  


### 2.1 The Base9 Compiler

Let's start by taking a look at [b9/src/Compiler.hpp]. This is where we define our `Compiler` class. 

[b9/src/Compiler.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/Compiler.hpp

The `Compiler` class constructor takes a `VirtualMachine` class (which we've already seen), and a `Config` struct. 

`Compiler(VirtualMachine &virtualMachine, const Config &cfg);`

The `Config` struct is in [b9/include/b9/VirtualMachine.hpp] and is defined as follows:

[b9/include/b9/VirtualMachine.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/VirtualMachine.hpp

```
struct Config {
  std::size_t maxInlineDepth = 0;  //< The JIT's max inline depth
  bool jit = false;                //< Enable the JIT
  bool directCall = false;         //< Enable direct JIT to JIT calls
  bool passParam = false;          //< Pass arguments in CPU registers
  bool lazyVmState = false;        //< Simulate the VM state
  bool debug = false;              //< Enable debug code
  bool verbose = false;            //< Enable verbose printing and tracing
};
```

We use the `Config` class to configure the JIT with a number of settings. Note that `directCall`, `passParam`, and `lazyVmState` are all JIT optimizations. We'll discuss them later. The rest of the values are explained in the comments. 

Let's have a closer look at the `Compiler` class:

```
class Compiler {
 public:
  Compiler(VirtualMachine &virtualMachine, const Config &cfg);
  JitFunction generateCode(const std::size_t functionIndex);

  const GlobalTypes &globalTypes() const { return globalTypes_; }

  TR::TypeDictionary &typeDictionary() { return typeDictionary_; }

  const TR::TypeDictionary &typeDictionary() const { return typeDictionary_; }

 private:
  TR::TypeDictionary typeDictionary_;
  const GlobalTypes globalTypes_;
  VirtualMachine &virtualMachine_;
  const Config &cfg_;
};
```

`TR::TypeDictionary` allows us to define which types are supported by our b9porcelain language. It's used by the `MethodBuilder` class in OMR JitBuilder. We define our supported types in [b9/include/b9/compiler/GlobalTypes.hpp].

[b9/include/b9/compiler/GlobalTypes.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/GlobalTypes.hpp

```c++
/// A collection of basic, built in types.
class GlobalTypes {
 public:
  GlobalTypes(TR::TypeDictionary &td);

  TR::IlType *addressPtr;
  TR::IlType *int64Ptr;
  TR::IlType *int32Ptr;
  TR::IlType *int16Ptr;

  TR::IlType *stackElement;
  TR::IlType *stackElementPtr;
  TR::IlType *instruction;
  TR::IlType *instructionPtr;

  TR::IlType *operandStack;
  TR::IlType *operandStackPtr;
  TR::IlType *executionContext;
  TR::IlType *executionContextPtr;
};
```

We use the `TR::TypeDictionary` to define our `GlobalTypes`, as shown above. 

Now let's have a look in [Base9/b9/src/Compiler.cpp] at the `Compiler::generateCode` function:

[b9/src/Compiler.cpp]: https://github.com/b9org/b9/blob/master/b9/src/Compiler.cpp

```c++
JitFunction Compiler::generateCode(const std::size_t functionIndex) {
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex);
  MethodBuilder methodBuilder(virtualMachine_, functionIndex);

  if (cfg_.debug)
    std::cout << "MethodBuilder for function: " << function->name
              << " is constructed" << std::endl;

  uint8_t *result = nullptr;
  auto rc = compileMethodBuilder(&methodBuilder, &result);

  if (rc != 0) {
    std::cout << "Failed to compile function: " << function->name
              << " nargs: " << function->nargs << std::endl;
    throw b9::CompilationException{"IL generation failed"};
  }

  if (cfg_.debug)
    std::cout << "Compilation completed with return code: " << rc
              << ", code address: " << static_cast<void *>(result) << std::endl;

  return (JitFunction)result;
}
```

The `generateCode` function takes a `functionIndex` as it's only parameter. First we need to access the current function using `virtualMachine_.getFunction(functionIndex)`. `getFunction()` is in the `VirtualMachine` class, and it uses the function index to access a given `FunctionDef` in the Module's function vector. We store it's return value in the function pointer `*function`. 

The checks `if(cfg_.debug)` are found throughout our codebase, and we use them to check if we're running the VM in debug mode. If we are, we provide additional output. It's a useful debugging strategy that you may or may not choose to employ. 

The `rc` value is set to 0 and should remain as 0 through the `compileMethodBuilder` call. If it does not remain 0, something has gone wrong. 

Our return value is simply a pointer to a uint8_t, which serves as the entry point into our Jitted function. 

`generateCode` is called by the VM function `generateAllCode`. 

Currently, we've only implemented the ability to JIT compile either everything or nothing. We run our program using the JIT with the following command: 

`./b9run/b9run -jit ./test/<program>.b9mod`


### 2.2 JIT Features

Currently, we've implemented 3 JIT optimizations. Direct call, Pass Param, and Lazy VM State. 

Direct call allows us to check whether or not the function we are calling has been JIT compiled, and then jump directly to the JITed function, bypassing the interpreter.

Pass Param allows JIT compiled methods calling other JIT compiled methods to pass their parameters using C native calling conventions. 
Lazy VM State simulates the interpreter stack while running in a compiled method and restores the interpreter stack when returning into the interpreter. 

Because of our current all-or-nothing state, if one method is JIT compiled, they all are, and using the above optimizations will improve performance significantly. To run the JIT with the above optimizations, do:

`./b9run/b9run -jit -directcall -passparam -lazyvmstate ./test/<program>.b9mod`


