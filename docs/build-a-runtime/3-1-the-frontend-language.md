---
layout: subchapter
title: The Frontend Language
---

## Frontend

{% include fig_img src="b9frontend.png" %}

### Frontend Language

Our frontend language is a primitive subset of JavaScript. Currently, it can only operate on integers. Let's have a look at some code:
 
 ```js
b9PrintString("Hello World!");
```

 `b9PrintString` is a method from a tiny library of functions that will be compiled with every program. The library is called b9stdlib and can be found in [b9/js_compiler/b9stdlib.js]. The functions in b9stdlib all call base9 [primitive functions]. In the Hello, World! program, Our script uses `b9PrintString` from b9stdlib to write text to console.

[test/hello.js]: https://github.com/b9org/b9/blob/master/test/hello.js
[primitive functions]: ../docs/Dictionary.md#primitive-function
[b9/js_compiler/b9stdlib.js]: https://github.com/b9org/b9/blob/master/js_compiler/b9stdlib.js

### Frontend Compiler

The frontend compiler is in [js_compiler/compiler.js]. It takes the source code and uses [Esprima] to convert the program into an [abstract syntax tree] (or AST). The frontend compiler walks the AST and converts it into a portable binary format. This portable binary format is represented as a [binary module].

[js_compiler/compiler.js]: https://github.com/b9org/b9/blob/master/js_compiler/compile.js
[frontend compiler]: https://github.com/b9org/b9/blob/master/js_compiler/compile.js
[Esprima]: http://esprima.org
[abstract syntax tree]: https://en.wikipedia.org/wiki/Abstract_syntax_tree
[binary module]: ../docs/Dictionary.md#binary-module

For a brief overview of the frontend compiler, as well as a more in depth look at the binary module, visit the link below:

[Frontend Compiler and Binary Format](/docs/FrontendAndBinaryMod.md)

Let’s convert the Hello, World! program to its binary format by running it through the frontend compiler. Hello, World! is in [b9/test/hello.js].

[b9/test/hello.js]: https://github.com/b9org/b9/blob/master/test/hello.js

From the root directory, run:

`node js_compiler/compile.js test/hello.js hello.b9mod`

The above command will run the frontend compiler on `test/hello.js` and output a binary module with the name `hello.b9mod`. If you run the above command, you'll see `hello.b9mod` in the base9 root directory. The `.b9mod` files are in raw hexadecimal format, and are legible using a hex editor like `dhex`.
