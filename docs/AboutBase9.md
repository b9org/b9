---
layout: about
title: About Base9
---

Base9 is an educational [language runtime] and JavaScript [interpreter]. It's front-end language is a subset of JavaScript with limited functionality. It compiles into a simple set of [bytecodes] which run on a primitive interpreter. We've also plugged in [OMR], a language-agnostic runtime toolkit, which provides our runtime with a [JIT compiler]! Plugging in OMR is made easy with the use of [JitBuilder], a easy-to-use tool that allows you to programmatically describe the semantics of your language's bytecodes.

[language runtime]: ./Dictionary.md#language-runtime
[bytecodes]: ./Dictionary.md#bytecode
[interpreter]: ./Dictionary.md#interpreter
[OMR]: https://www.eclipse.org/omr/
[JIT compiler]: ./Dictionary.md#jit-compiler
[JitBuilder]: https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/

Interested in building language runtimes? Check out our [Build your own Runtime] tutorial to learn how! Or if you want to learn more about base9, visit the [base9 overview section] of the tutorial. And be sure to visit our [documentation page] for a complete listing of our docs and tutorials.

[Build your own Runtime]: ./BuildARuntime.md
[base9 overview section]: ./BuildARuntime.md#base9-overview
[documentation page]: ./Documentation.md
