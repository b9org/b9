---
title: B9 Interpreter Operators
layout: page
---

This document is a WIP proposal for the complete bytecode set supported by Base9. Today, Base9 only supports a limited
few integer operations.

# B9 VM's Instruction Set

Base9 VM executes an untyped, stack-oriented, complex instruction set. In B9, there are three types of values:

* object reference
* double
* integer
* symbol
* builtin constants

Base9 supports the following built-in special values:

* undefined
* true
* false
* null

## Operator Listing

### Constants and literals
```lisp
(push-true)                 ;; ( -- true)
(push-false)                ;; ( -- false )
(push-null)                 ;; ( -- null )
(push-undefined)            ;; ( -- undefined )

(push-int-const integer)    ;; ( -- integer )
(push-double-const double)  ;; ( -- double )
(push-string id)            ;; ( -- string )
(push-symbol id)            ;; ( -- symbol )
(push-const-object id)      ;; ( -- object )
```

Constants are typed instructions. This allows us to abstract the encoding mechanism. Large constants, such as literal arrays or strings, are referenced as index
### Stack Operations
```lisp
(duplicate)
(drop n)
```

### Locals
```lisp
(push-from-local index)  ;; ( -- value )
(pop-into-local  index)  ;; ( value -- )
```

### Arguments
```lisp
(push-from-argument index)  ;; ( -- value )
(push-from-argument-i)      ;; ( index -- value )
(pop-to-argument index)     ;; ( value -- )
(push-arguments)            ;; ( -- arguments )
(push-rest)                 ;; ( -- rest )
```

### Globals
```lisp
(push-from-global id)
(pop-into-global  id)
```

### Environment Register
```lisp
(push-from-environment hops index)  ;; ( -- value )
(pop-into-environment  hops index)  ;; ( value -- )
(enter-environment nslots)          ;; ( -- )
(leave-environment)                 ;; ( -- )
```

The environment register is used to share state across function scopes. The environment is a chain 

### This Register
```lisp
(call-with-this)  ;; ( args* callee this -- result )
(push-this)       ;; ( -- this )
```

`this` is a register that refers to the current object.

By default, `this` is the global object.

### Symbolicating
```lisp
(coerce-string)  ;; ( value -- string* )
(coerce-symbol)  ;; ( value -- symbol )
```

### Object Manipulation
```lisp
(new-object)        ;; ( -- object )                    push a new empty object reference to stack
(push-from-object)  ;; ( slot-symbol object -- value )  indirect object load
(pop-into-object)   ;; ( value slot-symbol object -- )  indirect object store
```

### Call
```lisp
(call-function fn-index nparams)  ;; ( arg* -- result )
(call-primitive prim-index)     ;; ( arg* -- 0 )
(push-function fn-index)        ;; ( -- fn-object )            push fn-object
(call-object nparams)             ;; ( args* object -- result )  pop object and call as function
```

### Arithmetic
```lisp
(add)
(subtract)
(multiply)
(divide)       ;; ( lhs rhs -- result )
(power)        ;; ( x y -- result ) 
(mod)          ;; ( x y -- r )
(shift-left)   ;; ( nbits value -- result )
(shift-right)  ;; ( nbits value -- result )
(zshift-right) ;; ( nbits value -- result )
(bit-not)
(bit-and)
(bit-or)
(bit-xor)
```

* Throws TypeError on badly typed input

### Boolean
```lisp
(coerce-bool)  ;; ( value -- bool )
(not)     ;; ( value -- bool )
(and)     ;; ( value value -- bool )
(or)      ;; ( value value -- bool )
(xor)     ;; ( value value -- bool )
```

Boolean coercion can be explicitly performed, for equality tests against true.
Boolean operators implicity coerce their values to booleans.

The mapping is:

* True: object, non-empty-string, non-zero integer or double
* False: null, false, undefined, 0, ""


### Untyped Comparison
```lisp
(equal)        ;; ( lhs rhs -- bool )
(not-equal)
(less-than)
(greater-than)
(greater-or-equal-than)
(less-or-equal-than)
```

* Note that equal does not coerce values.

### Jump Operators
```lisp
(jmp target)
(jmp-if target)      ;; ( bool -- )
```

* Pop the top value of the stack and compare to true.
* Implicitly coerces the value to a boolean, as if (coerce-bool).

### Primitive Reflection
```lisp
(instance-of kind)  ;; ( thing -- bool )
```

where kind is one of:

| Code | Mmemonic |
|------|----------|
| 0    | object   |
| 1    | integer  |
| 2    | double   |
| 3    | symbol   |

### Misc Operators
```lisp
(nop)               ;; ( -- ) the best instruction
(breakpoint id)     ;; ( -- ) drop out to interpreter debug handler
(exit exit-code)    ;; ( -- ) terminate.
(throw)             ;; ( value -- ) Pop value and throw out to handler.
```
## Extension: Exception handling
see: https://github.com/WebAssembly/exception-handling/blob/master/proposals/Level-1.md

## Extension: Atomics?
see: https://llvm.org/docs/Atomics.html

## Extension: Inline caches
see: https://jandemooij.nl/blog/2017/01/25/cacheir/

# misc links
* https://en.wikipedia.org/wiki/Java_bytecode_instruction_listings
* https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Internals/Bytecode
* https://github.com/v8/v8/blob/master/src/interpreter/bytecodes.h
* https://www.gnu.org/software/guile/manual/html_node/Instruction-Set.html#Instruction-Set
