---
layout: default
title: About Base9
---

## Base9 Overview

Base9 is an educational [language runtime] and JavaScript [interpreter]. It's front-end language is a subset of JavaScript with limited functionality. It compiles into a simple set of [bytecodes] which run on a primitive interpreter. We've also plugged in [OMR], a language-agnostic runtime toolkit, which provides our runtime with a [JIT compiler]! Plugging in OMR is made easy with the use of [JitBuilder], a easy-to-use tool that allows you to programmatically describe the semantics of your language's bytecodes.

[language runtime]: ./Dictionary.md#language-runtime
[bytecodes]: ./Dictionary.md#bytecode
[interpreter]: ./Dictionary.md#interpreter
[OMR]: https://www.eclipse.org/omr/
[JIT compiler]: ./Dictionary.md#jit-compiler
[JitBuilder]: https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/

<figure class="image">
  <figcaption>Base9 Architecture</figcaption>
  <img src="./assets/images/b9architecture.png" width="100%"/>
</figure>

#### The Ahead of Time (AOT) Compilation Unit

The AOT compilation unit converts the base9 source code into a binary module containing bytecodes and metadata about the program. This module will be used as input to the VM at runtime.

#### The Virtual Machine / Runtime Unit
At runtime, the VM deserializes the binary module and converts it to an in memory data structure. We will refer to this data structure, henceforth, as the "in memory module". The bytecodes stored by the in memory module are then either interpreted or JIT compiled at runtime.

```

```

<center> *** </center>

```

```

Interested in building language runtimes? Check out our [Build your own Runtime] tutorial to learn how! And be sure to visit our [documentation page] for a complete listing of our docs and tutorials.

[Build your own Runtime]: ./BuildARuntime.md
[documentation page]: ./Documentation.md
