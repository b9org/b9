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
|---om/
|---test/
|---third_party/
```


### The b9/ directory

The `b9/` directory contains the Base9 core library.

```
|---b9/
|   |---include/
|   |   |---b9/
|   |   |   |---compiler/
|   |   |   |   |---Compiler.hpp
|   |   |   |   |---GlobalTypes.hpp
|   |   |   |   |---MethodBuilder.hpp
|   |   |   |   |---VirtualMachineState.hpp
|   |   |   |---ExecutionContext.hpp
|   |   |   |---OperandStack.hpp
|   |   |   |---VirtualMachine.hpp
|   |   |   |---assemble.hpp
|   |   |   |---binaryformat.hpp
|   |   |   |---deserialize.hpp
|   |   |   |---instructions.hpp
|   |   |   |---module.hpp
|   |   |   |---serialize.hpp
|   |---src/
|   |   |---Compiler.cpp
|   |   |---ExecutionContext.cpp
|   |   |---MethodBuilder.cpp
|   |   |---assemble.cpp
|   |   |---core.cpp
|   |   |---deserialize.cpp
|   |   |---primitives.cpp
|   |   |---serialize.cpp
```

Some important files to note:
- Our header files are in `b9/include/b9`
- ExecutionContext.cpp => contains the interpreter
- Compiler.cpp => contains the JIT
- VirtualMachine.hpp => contains the VM
- instructions.hpp => contains our bytecode definitions


### The b9assemble/ directory

The b9 assembler is a work in progress. When finished, it will take our [Base9 Assembly] and convert it to a binary module. 

[Base9 Assembly]: ./B9Assembly.md

```
|---b9assemble/
|   |---b9assemble.cpp
|   |---testAssemble.cpp
```


### The b9disassemble/ directory

You can read about the b9 disassembler at the [Base9 Disassembler page].

[Base9 Disassembler page]: ./Disassembler.md

```
|---b9disassemble/
|   |---testModules/
|   |---b9disassemble.cpp
|   |---testDisasDisaDisassemblee.cpp
```


### The b9run/ directory

The `b9run/` directory contains our `main` program. This is where we do our command line argument parsing, call the deserializer, and fire up the VM.

```
|---b9run/
    |--main.cpp
```


### The build/ directory

The `build/` directory is where we store our executables and build artifacts. It is not a part of our repository, but rather it is manually generated when we [setup Base9] in the Build Base9 section. 

[setup Base9]: ./SetupBase9.md#Build-Base9


### The docs/ directory

The `docs/` directory is where we store our markdown files, images, old presentations, and the files for our website. The markdown files can be viewed from our GitHub page, but have also been integrated into the [Base9 website].

[Base9 website]: https://b9org.github.io/b9/

```
|---docs/
|   |---_includes/
|   |---_layouts/
|   |---_sass/
|   |---assets/
|   |---images/
|   |---presentations/
|   |---404.html
|   |---AssemblerSyntax.md
|   |---B9Assembly.md
|   |---Blog.md
|   |---Deserializer.md
|   |---DeveloperJourney.md
|   |---Dictionary.md
|   |---DirectoryStructure.md
|   |---FrontEndAndBinaryMod.md
|   |---SetupBase9.md
|   |---SetupOSX.md
|   |---SetupUbuntu.md
|   |---index.md
```


### The js_compiler/ directory

The `js_compiler` directory is where we keep our [front-end compiler] and our [base9 primitives].

[front-end compiler]: ./FrontendAndBinaryMod.md
[base9 primitives]: https://github.com/b9org/b9/blob/master/js_compiler/b9stdlib.src

```
|---js_compiler/
|   |---b9stdlib.src
|   |---compile.js
```

### The om/ directory

The `om/` directory contains our Object Model Project, which is currently under development. 

```
|---om/
|   |---glue
|   |---include
|   |---src
|   |---test
```

### The /test directory

The `test/` contains example b9porcelain programs that we use for testing our front-end compiler.

```
|---test/
|   |---b9test.cpp
|   |---factorial.src
|   |---fib.src
|   |---hello.src
|   |---interpreter_test.src
|   |---simple_add.src
|   |---test.src
```


### The /third_party directory

The `third_party/` directory is where we store our submodules. This is where we keep the [OMR Repository], as well as [googletest].

[OMR Repository]: https://github.com/eclipse/omr
[googletest]: https://github.com/google/googletest
