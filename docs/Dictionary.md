---
layout: default
title: Base9 Dictionary
---

# Base9 Dictionary

Welcome to the Base9 Dictionary! This is where you can find definitions for terms and phrases used throughout the documentation.

### Binary Module
A binary module is a representation of some data in binary format. In our case, it is a representation of the in memory module, which is a C++ data structure. It encodes meta data about the module and the bytecodes of each function. Visit our [Binary Module] page to learn more.  

[Binary Module]: ./FrontendAndBinaryMod.md#binary-format

### Bytecode 
A bytecode is a type of instruction that can be consumed by a virual machine. Bytecodes are independent of a specific processor's instruction set, allowing for cross-platform portability. Bytecodes are designed to be efficiently decoded and interpreted by a computer program, and are not necessarily human readable. 

### Deserializer
The base9 deserializer is a program which takes a sequence of bytes and converts it to an in memory module which can be read and run by the VM. 

### Intermediate Language
An intermediate language is a compiler's intermediate representation between a high-level language and the native machine code. In the case of OMR, it occurs between the bytecodes and the native machine code, and it is tree based. Intermediate Languages are designed to be conducive for compiler optimizations.

### Interpreter 
An interpreter translates bytecodes into executable native machine code. The [base9 interpreter] is essentially a while loop surrounding a giant switch statement. Each `case` in the switch statement corresponds to a specific bytecode, and executes a corresponding C++ function to implement the functionality of that particular bytecode.

[base9 interpreter]: https://github.com/b9org/b9/blob/master/b9/src/ExecutionContext.cpp

### JIT Compiler 
A Just-In-Time (JIT) compiler is a runtime compilation system that compiles bytecodes by function into native machine code, and stores a pointer to the natively compiled code for future runs of the same function. Generally, the VM decides which functions to JIT compile based on profiling statistics. These statistics may also be used to further optimize the JIT compiled code.

### Language Runtime 
A language runtime is a piece of software that implements a programming language. It usually contains various runtime units, such as profiling, JIT compilation, code optimization, garbage collection, and more. A popular example of a language runtime is the Java Virtual Machine (JVM). 

### Primitive Function 
A function written in C++, but callable from b9porcelain source code. When calling a primitive function, it is indistinguishable from a b9porcelain function. Our base9 primitives are stored in our [b9_primitives table], and their definitions can be found in [b9/src/primitives.cpp]. 

[b9_primitives table]: https://github.com/b9org/b9/blob/master/js_compiler/b9stdlib.src
[b9/src/primitives.cpp]: https://github.com/b9org/b9/blob/master/b9/src/primitives.cpp

### Git Submodule 
A git submodule is a git repository within another git repository. It allows a developer to use project B (the submodule) from within project A. Project B is a subdirectory within project A, but is treated as a separate and independent git repository. To learn more about submodules, read the [submodules documentation] from git.

[submodules documentation]: https://git-scm.com/book/en/v2/Git-Tools-Submodules

### Virtual machine 
A program that takes a sequence of instructions as input and executes them. Programs given to a virtual machine are ahead-of-time compiled into bytecodes, and then interpreted. 
