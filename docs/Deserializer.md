---
layout: default
title: Deserializer
---

## The Base9 Deserializer

The deserializer takes a binary module as input, and outputs the Module Data structure. If we were to give it the binary module from our [binary module] example: 

[binary module]: ./FrontendAndBinaryMod.md

```
62 39 6d 6f 64 75 6c 65  01 00 00 00 01 00 00 00  04 00 00 00 66 75 6e 63
00 00 00 00 00 00 00 00  00 00 00 00 01 00 00 0d  02 00 00 0d 00 00 00 09
00 00 00 02 00 00 00 00  02 00 00 00 01 00 00 00  04 00 00 00 63 6f 64 65
```

... it would create a Module structure in C++ and populate it with the information from the binary module.

The deserializer is also used in the Base9 disassembler, which can be utilized for debugging. The disassembler is run with the following command:

`./b9disassemble/b9disassemble <binary_module>`

When run with the binary module from our demonstration, it will output:

```
(function "func" 2 2
  0  (int_push_constant 1)
  1  (int_push_constant 2)
  2  (int_add)
  3  (function_return)
  4  (end_section))

(string "code")
```

You can view the deserializer code in [Base9/b9/src/deserialize.cpp] and [Base9/b9/include/b9/deserialize.hpp]. 

[b9/src/deserialize.cpp]: https://github.com/b9org/b9/blob/master/b9/src/deserialize.cpp
[b9/include/b9/deserialize.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/deserialize.hpp
