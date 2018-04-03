---
layout: default
title: Base9 Disassembler
---

## The Base9 Disassembler

The disassembler employs the base9 deserializer. It takes a binary module as input, and outputs the in memory module to console in human readable form. If, for example, we were to disassemble the binary module from our [binary module example]: 

[binary module example]: ./FrontendAndBinaryMod.md#binary-module-example

```
62 39 6d 6f 64 75 6c 65  01 00 00 00 01 00 00 00  04 00 00 00 66 75 6e 63
00 00 00 00 00 00 00 00  00 00 00 00 01 00 00 0d  02 00 00 0d 00 00 00 09
00 00 00 02 00 00 00 00  02 00 00 00 01 00 00 00  04 00 00 00 63 6f 64 65
```

... it would create a c++ in memory Module, and output the following:

```
(function "func" 2 2
  0  (int_push_constant 1)
  1  (int_push_constant 2)
  2  (int_add)
  3  (function_return)
  4  (end_section))

(string "code")
```

The disassembler is a useful debugging tool. It can be run with the following command:

`./b9disassemble/b9disassemble <binary_module>`


You can view the deserializer code in [b9/src/deserialize.cpp] and [b9/include/b9/deserialize.hpp]. 

[b9/src/deserialize.cpp]: https://github.com/b9org/b9/blob/master/b9/src/deserialize.cpp
[b9/include/b9/deserialize.hpp]: https://github.com/b9org/b9/blob/master/b9/include/b9/deserialize.hpp
