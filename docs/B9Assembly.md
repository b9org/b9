---
layout: default
title: Base9 Assembly
---

# Base9 Assembly Example

```
(function "factorial" 1 0
  0  (push_from_var 0)
  1  (int_push_constant 0)
  2  (int_jmp_neq 2)
  3  (int_push_constant 1)
  4  (function_return)
  5  (push_from_var 0)
  6  (push_from_var 0)
  7  (int_push_constant 1)
  8  (int_sub)
  9  (function_call 3)
  10  (int_mul)
  11  (function_return)
  12  (end_section))
  ```
