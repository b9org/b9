---
layout: default
title: About Base9
---

## Base9 Architecture

Base9 is comprised of two major components:

#### 1. Ahead of Time (AOT) Compilation Unit

The AOT compilation unit is where the Base9 source code is tranformed into a binary module containing bytecodes and metadata about the program. This module will be used as input to the VM at runtime.

#### 2. Virtual Machine / Runtime Unit
At runtime, the binary module is given to the VM as input. The VM deserializes the module and converts it to an in memory data structure. We will refer to this data structure, henceforth, as the "in memory module". The in memory module will then be executed by the VM, either via interpretation, or JIT compilation. The interpreter is a part of Base9, whereas the JIT is provided by [OMR].

[OMR]: https://www.eclipse.org/omr/

<figure class="image">
  <img src="./images/b9Architecture.png" width="100%"/>
</figure>
