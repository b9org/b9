---
layout: subchapter
title: Compiling Functions with JitBuilder
---

### Run the JIT

As mentioned earlier, our JIT compiler is made possible by [OMR] and [JitBuilder]. OMR is kept in the [third_party] directory as a [git submodule]. JitBuilder is part of OMR. The JIT compiler is one of the two ways the VM can run the module (the other being the interpreter). Where the interpreter processes the bytecodes directly and one at a time, the JIT compiler converts the bytecodes to native machine code and returns a pointer to the start of the compiled code. Currently, we employ user flags to tell the VM to JIT compile an entire program and to interpret nothing. We aim to add profiling to base9, which will allow the VM to record statistics about the program and to make decisions about which individual functions to JIT compile.

[OMR]: https://eclipse.org/omr/
[JitBuilder]: https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/
[third_party]: https://github.com/b9org/b9/tree/master/third_party
[git submodule]: ../docs/Dictionary.md#git-submodule

So how do we run a fully JIT compiled program? From `build/` directory, run:

`./b9run/b9run -help`

You should see the following output:

```
Usage: b9run [<option>...] [--] <module> [<arg>...]
   Or: b9run -help
Jit Options:
  -jit:          Enable the jit
  -directcall:   make direct jit to jit calls
  -passparam:    Pass arguments in CPU registers
  -lazyvmstate:  Only update the VM state as needed
Run Options:
  -debug:        Enable debug code
  -verbose:      Run with verbose printing
  -help:         Print this help message
```

As you can see, there's a "Jit Options" section. Here we have a `-jit` option, as well as `-directcall`, `-passparam`, and `-lazyvmstate`. The latter three options are [advanced JIT features] that we'll explore later. For now, the `-jit` option is all we need. Let's run the JIT with our Hello, World! binary module.

[advanced JIT Features]: #jit-features

`./b9run/b9run -jit ../hello.b9mod`

The output should look exactly the same as when run with the interpreter. So what's the difference? The difference isn't really apparent when running a tiny program like Hello, World! The difference is apparent when re-running the same program multiple times, or running a very large program. We experienced about 9x speedup using the JIT, and that is excluding any of the [advanced JIT features].

[advanced JIT features]: #advanced-jit-features

### JIT Design

In general, compilers are vast and complex, with many layers and hidden depths. It can take years to become well versed in compiler technology, which discourages many developers from doing projects with them. That's why we built base9. We want to share the OMR technology with language developers and show them how adding a JIT compiler to a runtime can be easy! Let's start by taking a quick look at the JIT Design.

{%include fig_img src="jitOverview.png" %}

The above diagram shows the transition between the language bytecodes and the native machine code. The code must undergo several phases of transformation. The bytecodes are given to the [intermediate language (IL) generator] to be transformed into the IL. The IL is then further optimized via the [optimizer]. Optimized IL is then passed to the [code generator] for it's final converstion into [native machine code]. 

[intermediate language (IL) generator]: ../docs/Dictionary.md#il-generator
[optimizer]: ../docs/Dictionary.md#optimizer
[code generator]: ../docs/Dictionary.md#code-generator
[native machine code]: ../docs/Dictionary.md#native-machine-code

### Intermediate Language Generator and JitBuilder

In the above sections we covered the translation of JavaScript to bytecodes, and how base9 stores these bytecodes in a binary format before eventually deserializing them into the in-memory module. The above diagram shows the JIT taking the bytecodes as input, and returning native machine code as output. The first phase in this transition is IL Generation. Translating bytecodes to IL would normally be a big pain in the butt, but luckily, this is exactly what JitBuilder is for! JitBuilder is a tool for building an intermediate language from a set of bytecodes. Let's talk about how to use JitBuilder.

The first step is to call the `initializeJit()` function:

```cpp
if (cfg_.jit) {
  auto ok = initializeJit();
  if (!ok) {
    throw std::runtime_error{"Failed to init JIT"};
  }
}
```

We make this call in the `VirtualMachine` constructor in [b9/src/core.cpp]. `initializeJit()` sets up the OMR JIT by allocating a code cache for compiled methods. Conversely, the `VirtualMachine` deconstructor (also in [b9/src/core.cpp]) calls `shutdownJit()`, which frees the code cache of compiled methods:

[b9/src/core.cpp]: https://github.com/b9org/b9/blob/master/b9/src/core.cpp

```cpp
VirtualMachine::~VirtualMachine() noexcept {
  if (cfg_.jit) {
    shutdownJit();
  }
}
```

The next thing you'll need to consider for your runtime is the `MethodBuilder` class. `MethodBuilder` lives inside of OMR, but we've defined our own `MethodBuilder` class in base9 which inherits from the original. Let's open up [b9/include/b9/compiler/MethodBuilder.hpp] and have a look.

[b9/include/b9/compiler/MethodBuilder.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/MethodBuilder.hpp

Take a look at the base9 `MethodBuilder` class. It's fields are as follows:

```cpp
VirtualMachine &virtualMachine_;
const GlobalTypes &globalTypes_;
const Config &cfg_;
const std::size_t functionIndex_;
int32_t firstArgumentIndex = 0;
```

The `MethodBuilder` constructor takes the `VirtualMachine` and `functionIndex_` as parameters, and it sets the rest of the fields using existing data. We learned how to instantiate the `VirtualMachine` class in the [`run` function] section, and we know how to access the current function's index using [the `Module`] class's `getFunctionIndex` method. We've also already seen the `Config` struct in the [`main` function] section, but in case you forgot, we use the `Config` struct to configure the JIT with a number of settings, all of which are set upon running the VM. Recall running `./b9run/b9run -help`. All of the fields in the `Config` struct are configurable options that can be used when running the JIT.

[`run` function]: #the-run-function
[the `Module`]: #the-in-memory-module
[`main` function]: #the-main-function

Base9 uses a single array, `argsAndTempNames`, to store the arguments and temporaries of both the outer and inlined functions. The `firstArgumentIndex` variable is used to track and access from this array.

The final field to examine in `MethodBuilder` is `globalTypes_`. The `GlobalTypes` class uses `TR::TypeDictionary` to define the supported types. `TR::TypeDictionary` matches names of types to type data. JitBuilder expressions are typed, which means that any kind of operation is working with types. JitBuilder comes with a predefined set of basic types, corresponding to the fundamental types in C/C++. For example, JitBuilder provdes integer types of varying widths, doubles, and addresses. Jitbuilder allows us to define new types, derived from these builtins. For example, these facilities allow us to define pointer or struct types. 

See [b9/src/Compiler.cpp] for the `GlobalTypes` class, as shown below:

[b9/src/Compiler.cpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/GlobalTypes.hpp

```cpp
GlobalTypes::GlobalTypes(TR::TypeDictionary &td) {

  // Core Integer Types

  addressPtr = td.PointerTo(TR::Address);
  int64Ptr = td.PointerTo(TR::Int64);
  int32Ptr = td.PointerTo(TR::Int32);
  int16Ptr = td.PointerTo(TR::Int16);

  // Basic VM Data

  stackElement = td.toIlType<Om::RawValue>();
  stackElementPtr = td.PointerTo(stackElement);

  instruction = td.toIlType<RawInstruction>();
  instructionPtr = td.PointerTo(instruction);

  // VM Structures

  auto os = "b9::OperandStack";
  operandStack = td.DefineStruct(os);
  td.DefineField(os, "top_", td.PointerTo(stackElementPtr), OperandStackOffset::TOP);
  td.DefineField(os, "stack_", stackElementPtr, OperandStackOffset::STACK);
  td.CloseStruct(os);

  ...

}
```

`GlobalTypes` defines the various types we will be using in the base9 compiler. To start, we define a couple of common pointer-to-integer types, as well as a `StackElement` and `StackElementPtr` types. These types correspond to the types we've been using in C++. The last type we define, "b9::OperandStack", recreates the type information for the [OperandStack class], in the type dictionary. This gives JitBuilder expressions the ability to access fields inside the OperandStack. To create this struct type, we have to define the type and offset of each field.

[OperandStack class]: #the-operandstack

We've covered the `MethodBuilder` constructor, now we'll introduce `BytecodeBuilder`. Each `BytecodeBuilder` corresponds to one bytecode in the method. Each bytecode has an index associated with it, and from the index you can figure out the instruction, which bytecodes come next, which bytecodes come before it. Essentially, you can understand its position in the method.  

TODO RWY: What is a bytecode builder

Lets take a look in [b9/src/MethodBuilder.cpp]. Scroll down to the `generateILForBytecode` function: 

[b9/src/MethodBuilder.cpp]: https://github.com/b9org/b9/blob/master/b9/src/MethodBuilder.cpp

```cpp
bool MethodBuilder::generateILForBytecode(
    const FunctionDef *function,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    std::size_t instructionIndex,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {
  
  TR::BytecodeBuilder *builder = bytecodeBuilderTable[instructionIndex];
  const std::vector<Instruction> &program = function->instructions;
  const Instruction instruction = program[instructionIndex];

  TR::BytecodeBuilder *nextBytecodeBuilder = nullptr;

  if (instructionIndex + 1 <= program.size()) {
    nextBytecodeBuilder = bytecodeBuilderTable[instructionIndex + 1];
  }

  bool handled = true;

  switch (instruction.opCode()) {

    ...
    
    case OpCode::DUPLICATE: {
      auto x = pop(builder);
      push(builder, x);
      push(builder, x);
      if (nextBytecodeBuilder) {
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      }
    } break;
    
    ...
    
    case OpCode::DROP:
      drop(builder);
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    
    ...

    case OpCode::JMP:
      handle_bc_jmp(builder, bytecodeBuilderTable, program, instructionIndex, nextBytecodeBuilder);

    ...

    case OpCode::INT_ADD:
      handle_bc_add(builder, nextBytecodeBuilder);
      break;



```

Look familiar? Hopefully the above code snippet reminds you of the interpreter loop! It's very much the same. `generateILForBytecode` takes an entire method and compiles it into IL using `bytecodeBuilderTable`. The switch statement simply matches bytecodes with corresponding functionality. In the case of bytecode `DUPLICATE`, we pop a value from the builder and push the same value back onto the builder twice, thus duplicating the value. For `DROP`, we drop the top value off the builder. `JMP` and `INT_ADD` both call corresponding functions to handle their functionality.

`JMP` calls:

```cpp
void MethodBuilder::handle_bc_jmp(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *destBuilder = bytecodeBuilderTable[next_bc_index];
  builder->Goto(destBuilder);
}
```

`handle_bc_jmp` sets the `delta` variable to the jump target, `instruction.immediate() + 1`. It sets the next bytecode to the bytecode following the jump target. Note that the jump instruction does not contain the line `builder->AddFallThroughBuilder(nextBuilder);`. That's because it does not move to the next bytecode in the list incrementally.


`INT_ADD` calls:

```cpp
void MethodBuilder::handle_bc_add(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  push(builder, builder->Add(left, right));
  builder->AddFallThroughBuilder(nextBuilder);
}
```

`handle_bc_add` simply pops two values from the builder, calls `builder->Add` on the two values it just popped, and pushes the result back onto the builder. As with most of the other bytecodes, execution continues via `builder->AddFallThroughBuilder(nextBuilder);`.

One major difference between this switch statement and the interpreter's switch, is that the above `generateILForBytecode` function does not use a loop, but rather the function is called from inside of a loop in the `inlineProgramIntoBuilder` as follows:

```cpp
for (std::size_t index = GetNextBytecodeFromWorklist(); index != -1; 
     index = GetNextBytecodeFromWorklist()) {
  ok = generateILForBytecode(function, builderTable, index,
       jumpToBuilderForInlinedReturn);
  if (!ok) break;
}
```

### The `Compiler` class

The `Compiler` class the piece of the runtime that glues everything together. Let's take a look at [b9/src/Compiler.hpp], where we define the class: 

[b9/src/Compiler.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/Compiler.hpp

```cpp
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

Note the constructor. It takes a `VirtualMachine` class and a `Config` struct. Recall that we instantiated the `VirutalMachine` class in the base9 [`run` function]. We've also seen `Config` a couple of times (see the [`main` function] section for a refresher).

[`run` function]: #the-run-function
[`main` function]: #the-main-function

The first field is `TR::TypeDictionary`. It allows us to define which types are supported by the frontend language. We saw it in the [Intermediate Language Generator and JitBuilder] section, where it's used by the `MethodBuilder` class in OMR JitBuilder. We define our supported types in [b9/include/b9/compiler/GlobalTypes.hpp] using `TR::TypeDictionary`. 

[b9/include/b9/compiler/GlobalTypes.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/compiler/GlobalTypes.hpp
[Intermediate Language Generator and JitBuilder]: #[intermediate-language-generator-and-jitbuilder

### The `generateCode` function

Next we'll take a look in [b9/src/Compiler.cpp] at the `Compiler::generateCode` function:

[b9/src/Compiler.cpp]: https://github.com/b9org/b9/blob/master/b9/src/Compiler.cpp

```cpp
JitFunction Compiler::generateCode(const std::size_t functionIndex) {
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex);
  MethodBuilder methodBuilder(virtualMachine_, functionIndex);

  uint8_t *result = nullptr;
  auto rc = compileMethodBuilder(&methodBuilder, &result);

  return (JitFunction)result;
}
```

The `generateCode` function takes a `functionIndex` as it's only immediate. It start's by accessing the current function using `virtualMachine_.getFunction(functionIndex)`. Recall that `getFunction()` is part of the `VirtualMachine` class, and it uses the function index to access a `FunctionDef` in the Module's function vector. We store it's return value in the function pointer `*function`. 

The `rc` value is the return code of `compilerMethodBuilder`, and will return 0 on success.

The return value is simply a pointer to our newly Jitted function. 

`generateCode` is currently only called by the VM function `generateAllCode`,  because thus far we've only implemented the ability to JIT compile either everything or nothing.

### Advanced JIT Features

Currently, we've implemented 3 configurable advanced JIT features. Direct call, Pass Param, and Lazy VM State. 

Direct call allows us to check whether or not the function we are calling has been JIT compiled, and then jump directly to the JITed function, bypassing the interpreter.

Pass Param allows JIT compiled methods calling other JIT compiled methods to pass their parameters using C native calling conventions. 

Lazy VM State simulates the interpreter stack while running in a compiled method and restores the interpreter stack when returning into the interpreter. 

Because of our current all-or-nothing `-jit` option, if one method is JIT compiled, they all are, and using the above features will improve performance significantly.
