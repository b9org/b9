---
layout: chapter
title: Base9
subchapters:
  - 1-1-the-frontend-language.md
  - 1-2-bytecodes.md
  - 1-3-modules.md
  - 1-4-implementing-the-interpreter.md
  - 1-5-compiling-functions-with-jitbuilder.md
---

Before getting started, you should [get yourself set up]. We're going
to use it throughout the tutorial to explore and learn about language runtime
concepts. Don't forget to read [about base9] before getting started.

[get yourself setup with base9]: ../setup/index.md
[about base9]: ../about.md

## Base 9

Base9 has several major components that we'll discuss throughout the course of this tutorial. We'll provide insight about design decisions and implementation. Many of the design and implementation decisions were made based on the specific needs of the project. You may wish to deviate from these decisions along the way in order to best suit your own project.

{%include fig_img src="b9overview.png" %}

The above diagram details the two high-level components of the base9 architecture: the [ahead-of-time compilation] (AOT) and the [virtual machine] (VM) units. The AOT unit runs the frontend language through the frontend compiler (the base9 frontend language is a primitive subset of JavaScript). The frontend compiler outputs the [bytecodes] to be consumed by the VM. The VM (or runtime unit) will either execute tbe bytecodes one by one using the [interpreter], or it can employ the [JIT compiler] to produce optimized native machine code.

[ahead-of-time compilation]: ../docs/Dictionary.md#ahead-of-time-compilation
[virtual machine]: ../docs/Dictionary.md#virtual-machine
[bytecodes]: ../docs/Dictionary.md#bytecode
[interpreter]: ../docs/Dictionary.md#interpreter
[JIT compiler]: ../docs/Dictionary.md#jit-compiler

## OMR and JitBuilder

[OMR] is an open source and reusable C++ library for building language runtimes. It is designed to run on many different hardware and operating system platforms. It contains a JIT compiler, garbage collection, a platform porting library, a thread library, diagnostic services, and monitoring services. It's language agnostic functionality allows developers to easily bootstrap their own language runtimes. It is most notably employed in [OpenJ9], IBM's open sourced Java Virtual Machine, but has also been integrated with Ruby, CSOM, Lua, Swift, and Rosie Pattern Language.

[OMR]: https://www.eclipse.org/omr/
[OpenJ9]: https://www.eclipse.org/openj9/

{% include fig_img src="omrOverview.png" cap="OMR Overview" %}

The above diagram depicts the base9 components in yellow. These are the components that a developer must implement independantly. The red areas are the components belonging to OMR.

## JitBuilder

[JitBuilder] is an interface to the JIT compiler technology in OMR. It's designed to bootstrap a native-code JIT compiler for interpreted methods, and it allows the user to programatically describe the [intermediate language] (IL) that implements the semantics of the bytecodes. Using JitBuilder to employ the OMR JIT is not strictly necessary, but without it, one would require a deep understanding of JIT Compilation. JitBuilder makes it possible for someone without a background in compilers to easily plug-in and use the JIT compiler for their runtime.

[JitBuilder]: https://developer.ibm.com/open/2016/07/19/jitbuilder-library-and-eclipse-omr-just-in-time-compilers-made-easy/
[intermediate language]: ../docs/Dictionary.md#intermediate-language

