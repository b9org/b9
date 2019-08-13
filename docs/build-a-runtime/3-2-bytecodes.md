---
layout: subchapter
title: Bytecodes
---

## Backend

{%include fig_img src="b9backend.png" %}

Let's explore the VM design in greater detail.

{%include fig_img src="vmDesign.png" %}

The above diagram shows the components of the VM in detail. The VM takes the binary module and uses the deserializer to convert it into an in-memory Module containing the bytecodes. After the conversion, the VM will employ either the interpreter or the JIT to run the program. The interpreter processes the bytecodes directly and one at a time. The JIT converts the bytecodes to native machine code and returns a function pointer. Once a program is JIT compiled, the bytecodes are no longer interpreted. Instead, the JIT compiled version is always executed. Currently, when we run the JIT, we employ user flags to tell the VM to JIT compile an entire program and to interpret nothing.


### Bytecodes

The base9 instruction set is stack oriented, which allows for straight-forward compilation and simple VM implementation. All instructions operate on the operand stack, which can be thought of as the VM's memory. One advantage of a stack-based instruction set over a register-based model is that stack-based instructions are smaller, with no need for a register immediate. One disadvantage is that the total number of instructions is larger. For more information on the difference between stack and register based virtual machines, you can [read this article on the internet].

[read this article on the internet]: https://markfaction.wordpress.com/2012/07/15/stack-based-vs-register-based-virtual-machine-architecture-and-the-dalvik-vm/

All of the base9 bytecodes are fixed-width. This puts constraints on what we can encode in the instructions, but it simplifies instruction decoding and jumps.

Finally, the base9 instruction set is untyped. This means that the bytecodes can operate on values of varying types. The way this works is by popping the operands off the operand stack, checking their types, and doing the correct operation once the type is known.  

All base9 bytecodes are defined in [b9/include/b9/instructions.hpp].

[b9/include/b9/instructions.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/instructions.hpp

### The Operand Stack

As mentioned, the base9 bytecodes are stack oriented. Let's take a look at what happens with the operand stack during a simple addition function:

```js
function simple_add() {
  return 5 + 6;
} 
```

The following diagrams display the bytecodes generated from the "simple_add()" function. 

**IP** = Instruction Pointer

**SP** = Stack Pointer

Push 5 onto the operand stack:

{%include fig_img src="bcStack1.png" %}

Push 6 onto the operand stack:

{%include fig_img src="bcStack2.png" %}

Pop 5 and 6 off that stack, add them, push the result to the operand stack:

{%include fig_img src="bcStack3.png" %}

Pop and return the result from the operand stack:

{%include fig_img src="bcStack4.png" %}

### The Deserializer

The base9 [deserializer] at [b9/src/deserialize.cpp] is responsible for taking the binary module and converting it to the in-memory Module. The deserializer is used in base9 in two different ways. Firstly, it's used by the VM to convert a binary module to an in-memory Module. Secondly, it is used by the disassembler at [b9/b9disasm/b9disasm.cpp]. The disassembler employs the deserializer to convert a binary module into an assembly-like interpretation, which we're calling [base9 assembly]. It's primarily used for debugging. Click the link below to learn more:

[deserializer]: ../docs/Dictionary.md#deserializer
[b9/src/deserialize.cpp]: https://github.com/b9org/b9/blob/master/b9/src/deserialize.cpp
[b9/b9disasm/b9disasm.cpp]: https://github.com/b9org/b9/blob/master/b9disasm/b9disasm.cpp
[base9 assembly]: ../docs/B9Assembly.md

[Base9 Disassembler](../docs/Disassembler.md)

Let's run the disassembler using the binary module we generated in the [Frontend Compiler section]! From the `build/` directory, run the following command:

[Frontend Compiler section]: #frontend-compiler

`b9disasm/b9disasm ../hello.b9mod`

You should now be looking at a human readable version of the Hello, World! program as represented by [base9 assembly]. You'll notice that the first three functions (`b9PrintString`, `b9PrintNumber`, and `b9PrintStack`) are the b9stdlib functions that are included in each compiled program. They can be ignored. The important part is the `<script>` function, which is the special script-body function, and the main entry to our program. Let's have a look at the transition between the JavaScript and the base9 assembly:

[base9 assembly]: ../docs/Dictionary.md#base9-assembly

```js
b9PrintString("Hello World!");
```

{%include fig_img src="downArrow.png" %}

```
(function "<script>" 0 0
  0  (str_push_constant 0)
  1  (function_call 0)
  2  (drop)
  3  (int_push_constant 0)
  4  (function_return)
  5  (end_section))
```

Stepping through the bytecodes as represented by the base9 assembly:
- `str_push_constant` pushes the string "Hello, World!" onto the operand stack
- `function_call` does the call to `b9PrintString`
- `drop` drops the top value from the operand stack (which was holding the unused return value of `b9PrintString`)
- `int_push_constant` pushes 0 onto the operand stack as the return value of our script.
- `function_return` does the actual return
- `end_section` is the end-of-bytecodes marker, a safety feature should the interpreter continue running after the return

