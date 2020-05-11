---
layout: subchapter
title: Implementing the Interpreter
---

### The Virtual Machine

Now that we've learned a bit about the bytecodes, let's run the VM with the Hello, World! binary module that we generated earlier. From the `build/` directory, run the following command: 

`b9run/b9run ../hello.b9mod`

You should see the following output:

```
Hello World!

=> (integer 0)
```

The `=> (integer 0)` is the return code of the program.

### The Interpreter

The interpreter's job is to take a sequence of bytecodes and match each of them with a corresponding C++ function. This relationship is one-to-one. Earlier, when we ran our Hello, World! program, it ran by default on the interpreter. The VM will always run the interpreter by default. The base9 interpreter is very simple, consisting of a while-loop and switch statement to iterate a sequence of bytecodes. Let's take a look at some pseudocode for the interpreter:

```c++
interpreter (function) {
  for all bytecodes in the function {
    switch(bytecode) {
      case functionCall:
        doFunctionCall();
      case functionReturn:
        doFunctionReturn();
      case integerAdd:
        dointegerAdd();
      case integerSubtract:
        doIntegerSubtract();
  
      ...

    }
  }
}
```

Hopefully the pseudocode has convinced you that the interpreter is actually super simple. Essentially, it's a loop around a switch statement, matching bytecodes to corresponding C++ functions.

## Implementation

### The `main` Function

Recall the command for running the VM with a binary module (from the `build/` directory):

`b9run/b9run ../hello.b9mod`

The entry point for `b9run` can be found in [b9run/main.cpp]. Let's take a look a the `main` function: 

[b9run/main.cpp]: https://github.com/b9org/b9/blob/master/b9run/main.cpp

```c++
int main(int argc, char* argv[]) {
  Om::ProcessRuntime runtime;
  RunConfig cfg;

  if (!parseArguments(cfg, argc, argv)) {
    std::cerr << usage << std::endl;
    exit(EXIT_FAILURE);
  }

  // ... 

  try {
    run(runtime, cfg);
  }

  // ...

  exit(EXIT_SUCCESS);
}
```

The first thing that occurs in `main` is the instatiation of `ProcessRuntime` and `RunConfig`. `ProcessRuntime` does a process wide initialization for the `om` libary, which provides OMR with facilities for garbage collected objects. No need to worry about it, because other than the instantiation, it takes care of itself!

`RunConfig` is a struct (inside of [b9run/main.cpp]), which sets up the base9 global configuration:

```c++
struct RunConfig {
  b9::Config b9;
  const char* moduleName = "";
  const char* mainFunction = "<script>";
  std::size_t loopCount = 1;
  bool verbose = false;
  std::vector<b9::StackElement> usrArgs;
};
```

Note the `Config` struct in `RunConfig`: 

```cpp
struct Config {
  bool jit = false;                //< Enable the JIT
  bool directCall = false;         //< Enable direct JIT to JIT calls
  bool passParam = false;          //< Pass arguments in CPU registers
  bool lazyVmState = false;        //< Simulate the VM state
  bool debug = false;              //< Enable debug code
  bool verbose = false;            //< Enable verbose printing and tracing
};
```

When we run `b9run`, we can run it with a number of options. The command structure for `b9run` is as follows:

`b9run [<option>...] [--] <module> [<arg>...]`

Try running `b9run/b9run -help` from the build directory.

The `RunConfig` and `Config` structs have a default configuration, but this default may be overridden according to the command line options provided by the user. All of the options listed using the `-help` option reside in either the `RunConfig` or the `Config` struct. We use these options to configure the JIT with a number of settings at startup.

Back to the `main` function, the next thing to occur is the argument parsing. This is where `RunConfig` and `Config` may be modified. 

The final thing to occur in `main` is a call to `run(runtime, cfg)`. We wrapped this call in a `try` block, because there are plenty of things that could go wrong when running the VM, and proper error handling is next to godliness.

### The `run` Function

Let's look at the `run` function called by `main`:

```cpp
static void run(Om::ProcessRuntime& runtime, const RunConfig& cfg) {
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

`run` is where the `VirtualMachine` class is instantiated. `VirtualMachine` can be found in [b9/include/b9/VirtualMachine.hpp]. The `run` function deserializes a binary module which has been compiled from JavaScript source code, and loads the resulting in-memory Module into the VM. Next, it checks if the JIT has been turned on. If yes, the bytecodes are JIT compiled using the `generateCode` function. If no, the VM obtains the main function of the program and begins interpreting.

### The Loaded `Module`

As mentioned, the Module loaded into memory by deserializing a [binary module]. Let's have a look at the `Module` class:

```cpp
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

The Module contains only 2 fields; a `functions` vector, and a `strings` vector. `functions` contains a set of function definitions, or `FunctionDef`'s. The `strings` vector in the Module contains the contents of the string table, which holds any strings used by the JavaScript program. Lastly, there's `getFunctionIndex`, which traverses the `functions` vector using a function name, and returns that function's position in the vector. 

### The `functionDef` struct

Now let's look at our `FunctionDef` struct: 

```cpp
struct FunctionDef {

  ...

  // Function Data
  std::string name;
  uint32_t index;
  std::uint32_t nparams;
  std::uint32_t nlocals;
  std::vector<Instruction> instructions;
};
```

The fields of the `FunctionDef` are the function name, the index in the function vector, the number of arguments, the number of registers, and the `Instructions` vector (which contains the bytecodes).

### The base9 Instruction Set

The base9 VM executes it’s own instruction set. The instructions are compiled from the frontend language. Each instruction is a 32-bit little-endian value, which encodes an opcode (AKA a bytecode), and an optional immediate. The opcode is the most significant byte of the instruction. It tells the VM which functionality to execute. The opcode is one of the special predefined constants that the VM understands. Example opcodes are `int_add`, `int_sub`, `jmp_eq`, and `function_return`. Each opcode corresponds with a unique hexadecimal value between 0 and n, where n is the total number of bytecodes.

The immediate encodes the constant data about an instruction. For example, a jump instruction encodes it’s jump target in the immediate. The immediate is a signed value stored in the lower 24 bits of the instruction.

The layout of the instructions is:

```
|0000-0000 0000-0000 0000-0000 0000-0000
|---------| bytecode (8 bits)
          |-----------------------------| immediate (24 bits)
```

Instructions are encoded as:

```cpp
std::uint32_t instruction := (opcode << 24) | (immediate &0xFFFFFF);
```

To decode an instructions opcode:

```cpp
std::uint8_t opcode = instruction >> 24;
```

Decoding the immediate is a little more involved. Since the immediate is signed, we must sign extend the 24-bit value as we convert it to 32 bits:

```cpp
std::uint32_t immediate = instruction & 0xFFFFFF;
bool is_signed = immediate & (1 << 23);
if (is_signed)
  immediate |= 0xFF << 24;
```

### The `OperandStack`

The `OperandStack` class is what we have previously referred to as [the operand stack]. It's definition can be found in [b9/include/b9/OperandStack.hpp]. The `OperandStack` class contains functions for using/manipulating the operand stack.

[the operand stack]: #the-operand-stack
[b9/include/b9/OperandStack.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/OperandStack.hpp

Values pushed and popped from the stack must be of type `StackElement`: 

```cpp
using StackElement = Om::Value;
```

The StackElement type comes from Om, an OMR toolkit for dealing with garbage collected objects. A `StackElement` is a 64-bit value that encodes a type and a payload. The value can hold a double, a GC reference, a valid memory pointer, or a 32-bit integer.

```cpp
StackElement *top_;
StackElement stack_[SIZE];
```

The operand stack acts as the VM's memory bank, storing all of the values needed by the instructions. To see how the operand stack operates during a simple add function, see [the operand stack] section above. As you've likely noticed, base9 is using a `StackElement` type, which is defined in [b9/include/b9/OperandStack.hpp] as follows:

[b9/include/b9/OperandStack.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/OperandStack.hpp

### The Interpreter Loop

The interpreter is implemented in [b9/src/ExecutionContext.cpp]. It uses the `OperandStack` to store the values needed by the instructions. When a function call occurs, the arguments to the function are pushed onto the operand stack before entering the function. Once the function is entered, the arguments are popped off and used. Let's take a look at the code:

[b9/src/ExecutionContext.cpp]: https://github.com/b9org/b9/blob/master/b9/src/ExecutionContext.cpp

```cpp
  auto function = virtualMachine_->getFunction(functionIndex);
  auto paramsCount = function->nparams;
  auto localsCount = function->nlocals;
  auto jitFunction = virtualMachine_->getJitAddress(functionIndex);

  if (jitFunction) {
    return callJitFunction(jitFunction, paramsCount);
  }

  // interpret the method otherwise
  const Instruction *instructionPointer = function->instructions.data();

  StackElement *params = stack_.top() - paramsCount;
  
  stack_.pushn(localsCount); //make room for locals in the stack
  StackElement *locals = stack_.top() - localsCount;
  ...

```

As you can see, the second last line is popping the functions arguments off the operand stack. Let's back up. The first bit of the `interpret` function deals with some initial setup. The single immediate is `functionIndex`, which we use to get the individual function we wish to interpret. The interpreter then checks if the particular function has previously been JIT compiled. If it has, we use that function instead, and return from the interpreter. If it hasn't, the `instructionPointer` is initialized to the start of the instruction sequence, the arguments are collected from the top of the operand stack, and storage for local variables is allocated on the operand stack.

Now let's take a look at the while-loop and switch-statement.

```cpp
  while (*instructionPointer != END_SECTION) {
    switch (instructionPointer->opCode()) {

      ...

      case OpCode::FUNCTION_RETURN: {
        auto result = stack_.pop();
        stack_.restore(args);
        return result;
        break;
      }

      ...

      case OpCode::INT_ADD:
        doIntAdd();
        break;

     ...

```

We've excluded much of the interpreter loop for simplicity, but we'll discuss a couple of the cases to give you an idea of how it works. Above we have the `FUNCTION_RETURN` bytecode case and the `INT_ADD` bytecode case. In the `FUNCTION_RETURN` case, the top of the operand stack (which is storing the return value of the function) is popped and stored in the `result` variable. The operand stack is then restored and the return value is returned. This is the only bytecode in the interpreter loop that returns a value, which makes sense, because this bytecode is only reached at the end of a function when there are no more bytecodes to process. The `INT_ADD` bytecode case calls `doIntAdd()`, which is a simple C++ function:

```cpp
void ExecutionContext::doIntAdd() {
  std::int32_t right = stack_.pop().getInt48();
  std::int32_t left = stack_.pop().getInt48();
  StackElement result;
  result.setInt48(left + right);
  push(result);
}
```

In summary, the interpreter is simply a mechanism for translating the bytecodes into C++. It’s one-at-a-time bytecode processing makes it inherently slow, which makes the JIT compiler an important part of reducing execution time.


### Summary

The VM has two ways of handling the Module. It can either execute the Module's bytecodes line by line in the interpreter, or it can choose to JIT compile the bytecodes and run that version instead. The interpreter processes the bytecodes directly and one at a time. The JIT compiler converts the bytecodes to native machine code and returns a pointer to the start of the code.

To run the VM using just the interpreter:

`./b9run/b9run <binary_module>`

A reminder that the above command should be run from the `/build` directory.

To conclude this section, let's briefly walk over the components we've covered thus far. JavaScript is the frontend language. It is compiled using the frontend compiler, which employs Esprima. The output of the frontend compiler is the binary module. The binary module is consumed by the deserializer in [b9/src/deserialize.cpp], which converts it to the in-memory Module to be run by the VM. The VM is itself a C++ class, and can be found in [b9/include/b9/VirtualMachine.hpp]. Once instantiated, the VM loads and runs the Module through the interpreter in [b9/src/ExecutionContext.cpp].

[b9/src/deserialize.cpp]: https://github.com/b9org/b9/blob/master/b9/src/deserialize.cpp
[b9/include/b9/VirtualMachine.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/VirtualMachine.hpp
[b9/src/ExecutionContext.cpp]: https://github.com/b9org/b9/blob/master/b9/src/ExecutionContext.cpp
