# Build Your Own Language Runtime


## Introduction

Welcome to our tutorial! If you're interested in building your own language runtime, you've come to the right place. Base9 is a miniature language runtime, and thanks to OMR and JitBuilder, it even has a **Just In Time (JIT) Compiler**! The goal of this tutorial is not to teach you how to build Base9, but rather to pack your arsenal full of tools to build your own language runtime. We don't want to bog you down with a bunch of unnecessary information, so we'll keep things very straightfoward, providing useful links/definitions along the way for you to peruse optionally and at your own convenience. Lets get started!


## A Brief Overview of Base 9

If you want to fiddle around with Base9, check out our [set-up page](https://github.com/arianneb/Base9/blob/documentation/doc/README.md) for set-up instructions. Otherwise, here's the short-winded description: Base9 is an educational **language runtime**. It's front-end language, b9porcelain, is a small subset of JavaScript. It compiles into a simple set of **bytecodes** which run on a primitive **interpreter**. We've also plugged in [OMR](https://www.eclipse.org/omr/) and [JitBuilder](https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/) to provide our runtime with a **JIT compiler**! 


## Creating Base9

Base9 has several major components that we'll discuss and demonstrate how we designed and built. We'll provide you with the steps we took, but we encourage you to remember that much of our implementation is made up of design decisions suited to our project. Along the way, you may wish to deviate from these decisions in order to best suit your own project.

### Base9 Overview:

![](https://github.com/arianneb/Base9/blob/developerQuest/images/b9overview.png)

The Base9 Overview Diagram depicts the Ahead-of-Time compilation unit and the Virtual Machine unit. The Ahead-of-Time unit runs the b9porcelain source code through our frontend compiler. The frontend compiler outputs a binary module. The binary module is passed to the deserializer, which converts it to an in memory Module.

The Virtual Machine unit is comprised of the Interpreter and the JIT (not depicted here). The in memory Module is passed to the VM, which runs the bytecodes of the program. 

We'll discuss each of the above components in detail in the upcoming sections. 


### b9porcelain

b9porcelain is our front-end language. As mentioned, it's a subset of JavaScript. Let's have a look at the code: 
 
 ```
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

Above is a classic program that we all know and love, Fibonacci. `b9main()` is using two **primitive functions** (from it's primitive function table), `b9PrintString` and `b9PrintNumber`. Currently, b9porcelain is only capable of operating on integers. It can output strings, but it can't (yet) perform operations on them.

b9porcelain source code is passed as input into our [frontend compiler](https://github.com/b9org/b9/blob/master/js_compiler/compile.js), which uses [Esprima](http://esprima.org) to convert the program into an [Abstract Syntax Tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree) (or AST). Our frontend compiler walks the AST and converts it into a binary module. Click the link for a more detailed overview of our [frontend compiler and binary module](https://github.com/arianneb/Base9/blob/developerQuest/doc/FrontendAndBinaryMod.md).


### Building the Module

The Base9 [deserializer](https://github.com/arianneb/Base9/blob/developerQuest/doc/Deserializer.md) is responsible for taking the binary module (which is output by the frontend compiler), and converting it into the Module data structure (containing the Base9 bytecodes) which can be run by the VM. The Module is represented in Base9 as follows:

```
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

The Module contains only 2 fields; a `functions` vector, and a `strings` vector. `functions` contains a set of function definitions, or `FunctionDef`'s:

```
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

The strings vector in the Module contain the contents of the string table, which holds any strings used by the b9porcelain program. 

Lastly in the Module, we see a function called `getFunctionIndex`, which traverses the functions vector using a function name, and returns that functions position in the vector. 


### Execution Model

The Base9 Virtual Machine is a relatively simple C++ class. It loads a Module structure which has been compiled from b9porcelain source code, and runs the program. The Module is created by converting b9porcelain into an executable sequence of bytecodes represented in binary format. The binary format is then deserialized into the Module structure and passed to the VM. The VM then has two ways of handling the Module. It can either execute the Module's bytecodes line by line in the interpreter, or it can choose to JIT compile the bytecodes and run that version instead. 

### b9porcelain to Bytecode:

![](https://github.com/arianneb/Base9/blob/developerQuest/images/b9porcelainToBC.png)

The above diagram depicts the high-level translation from b9porcelain to Base9 bytecodes. 


### Virtual Machine Design:

![](https://github.com/arianneb/Base9/blob/developerQuest/images/vmDesign.png)

The above diagram shows our Virtual Machine Design. The VM takes the bytecodes from the Module, and runs them through either the Interpreter or the JIT compiler. The Interpreter will process the bytecodes directly, and the JIT compiler converts them to native machine code. 

### The Virtual Machine

The Base9 VM has been subject to many design decisions. Please note, as with many of our components, there are multiple ways to implement a language runtime. As discussed above, the Base9 virtual machine takes a Module structure as input. Let's take a look at our [main function](https://github.com/b9org/b9/blob/master/b9run/main.cpp) to gain a better understanding of how this works. Our main function calls `run(runtime, cfg);`:

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

The `<module>` argument is the binary module, which we created using `compile.js`. If you take a look in [main.cpp](https://github.com/b9org/b9/blob/master/b9run/main.cpp), you'll notice our argument parsing function, which collects the `<module>` argument from the command line. If you choose to run your VM from the command line, you will need to do something similar. 

Again, how we structure and handle our binary format was a personal design decision that may or may not work for you. We chose to read our binary file into our in memory Module, and to load it into the VM to run. Lets take a closer look at how the VM runs our Module. 

### The Interpreter

Once the VM is instantiated and has loaded the Module, it calls the `VirtualMachine::run` function in [Base9/b9/src/core.cpp](https://github.com/b9org/b9/blob/master/b9/src/core.cpp), which in turn calls the interpreter. The interpreter can be found in [Base9/b9/src/ExecutionContext.cpp](https://github.com/b9org/b9/blob/master/b9/src/ExecutionContext.cpp) in the `interpret` function (`StackElement ExecutionContext::interpret(const std::size_t functionIndex)`). The `interpret` function contains a while loop wrapped around a switch statement. The while loop iterates the bytecodes in a given function and executes the appropriate code for each particular bytecode it encounters. 


### Base9 Summary

To conclude this section, let's briefly walk over the components we've covered thus far. `b9porcelain` is our front-end language. It is compiled by our [compiler](https://github.com/b9org/b9/blob/master/js_compiler/compile.js) using Esprima, and eventually converted into our binary module. The binary module is fed to our [deserializer](https://github.com/b9org/b9/blob/master/b9/src/deserialize.cpp), which converts it into the Module structure to be consumed by the VM. The VM is itself a C++ class, and can be found in [VirtualMachine.hpp](https://github.com/b9org/b9/blob/master/b9/include/b9/VirtualMachine.hpp). Once instantiated, the VM loads and runs the Module through the [interpreter](https://github.com/b9org/b9/blob/master/b9/src/ExecutionContext.cpp). 


## Plugging in the JIT Compiler 

As we mentioned earlier, our JIT compiler is made possible by [OMR](https://www.eclipse.org/omr/) and [JitBuilder](https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/). OMR is a submodule in our Base9 repository. We keep it in our [third_party](https://github.com/b9org/b9/tree/master/third_party) directory. JitBuilder is part of OMR. With OMR as a submodule, and with the correct `#include`'s in our Base9 headers, using OMR and JitBuilder functionality is easy!  

Let's start by taking a look at [Compiler.hpp](https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/Compiler.hpp). This is where we define our `Compiler` class. The constructor takes a `VirtualMachine` class (which we've already seen), and a `Config` struct. 

`Compiler(VirtualMachine &virtualMachine, const Config &cfg);`

The `Config` struct is in [VirtualMachine.hpp](https://github.com/b9org/b9/blob/master/b9/include/b9/VirtualMachine.hpp) and is defined as follows:

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

We use this class to configure the JIT with a number of settings. Note that `directCall`, `passParam`, and `lazyVmState` are all JIT optimizations that we'll discuss later. The rest of the values are explained in the comments. 

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

`TR::TypeDictionary` allows us to define which types are supported by our b9porcelain language. It's used by the `MethodBuilder` class in OMR JitBuilder. We define our supported types in [GlobalTypes.hpp](https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/GlobalTypes.hpp).

```
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

Now let's have a look in [Base9/b9/src/Compiler.cpp](https://github.com/b9org/b9/blob/master/b9/src/Compiler.cpp) at the `Compiler::generateCode` function:

```
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

The `generateCode` function takes a `functionIndex` as it's only parameter. First we need to access the current function using `virtualMachine_.getFunction(functionIndex)`. `getFunction()` is in the `VirtualMachine` class, and it uses the function index to access a given `FunctionDef`. We store it's return value in the function pointer `*function`. 

The checks `if(cfg_.debug)` are found throughout our codebase, and we use them to check if we're running the VM in debug mode. If we are, we provide additional output. It's a useful debugging strategy that you may or may not choose to employ. 

The `rc` value is set to 0 and should remain as 0 through the `compileMethodBuilder` call. If it does not remain 0, something has gone wrong. 

Our return value is simply a pointer to a uint8_t, which serves as the entry point into our Jitted function. 

`generateCode` is called by the VM function `generateAllCode`. 
