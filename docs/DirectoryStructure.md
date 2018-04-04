---
layout: default
title: Base9 Directory Structure
---

# Base9 Directory Structure 

Lets take a moment to get familiar with the directory structure of Base9. We'll leave out certain, less relevant, artifacts, like those belonging to `cmake` and `docker`, to simplify the discussion. 


### Base9 layout 

```
b9/
|---b9/
|---b9disassemble/
|---b9assemble/
|---b9docker/
|---b9run/
|---cmake/
|---docker/
|---docs/
|---js_compiler/
|---test/
|---third_party/
```


### The b9/ directory

The `b9/` directory contains the Base9 core library.

```
|---b9/
|   |---include/
|   |   |---b9/
|   |---src/
```

Note:
- The header files are in `b9/include/b9`
  - Bytecode definitions => b9/include/b9/instructions.hpp
- The source files are in `b9/src/`
  - The Interpreter => ExecutionContext.cpp
  - The JIT => Compiler.cpp
  - The VM => VirtualMachine.hpp


### The b9assemble/ directory

The b9 assembler is a work in progress. When finished, it will take our [Base9 Assembly] and convert it to a binary module. 

[Base9 Assembly]: ./B9Assembly.md


### The b9disassemble/ directory

You can read about the b9 disassembler at the [Base9 Disassembler page].

[Base9 Disassembler page]: ./Disassembler.md


### The b9run/ directory

The `b9run/` directory contains our `main` program. This is where we do our command line argument parsing, call the deserializer, and fire up the VM.


### The build/ directory

The `build/` directory is where we store our executables and build artifacts. It is not a part of our repository, but rather it is manually generated when we [build base9]. 

[build base9]: ./SetupBase9.md#build-base9


### The docs/ directory

The `docs/` directory is where we store our documentation, images, old presentations, and the files for our website. The markdown files can be viewed from our GitHub page, but have also been integrated into the [Base9 website].

[Base9 website]: https://b9org.github.io/b9/


### The js_compiler/ directory

The `js_compiler` directory is where we keep our [front-end compiler] and our [base9 primitives].

[front-end compiler]: ./FrontendAndBinaryMod.md
[base9 primitives]: https://github.com/b9org/b9/blob/master/js_compiler/b9stdlib.src


### The /test directory

The `test/` contains example b9porcelain programs that we use for testing our front-end compiler.


### The /third_party directory

The `third_party/` directory is where we store our submodules. This is where we keep the [OMR Repository], as well as [googletest].

[OMR Repository]: https://github.com/eclipse/omr
[googletest]: https://github.com/google/googletest
