---
title: Frontend Compiler and Binary Format
layout: page
---

## Frontend Compiler

The frontend compiler takes the JavaScript source code and, using the [Esprima] framework, parses it into an Abstract Syntax Tree (AST). Although Esprima handles a significant chunk of the work, we still need to parse the AST in order to create the bytecodes. AST parsing requires an understanding of AST's, as well as of the various node types output by Esprima. This understanding is not required for the tutorial, but for informational purposes only, the AST is recursively processed, and each node type is handled by a corresponding function. The frontend compiler eventually outputs a binary module, which can later be deserialized and run by the VM.

[Esprima]: http://esprima.org

To run the front-end compiler on a JavaScript program:

`node ./compile.js <in> <out>`

Where `<in>` is the name/path of the JavaScript program, and `<out>` is the name we'll choose for the binary module.


## Binary Format

Please note that the decision to convert the JavaScript into a binary module was a design decision. It allows for cross platform compatibility, and gives users the option of packaging binary modules instead of JavaScript source programs (which must first be run through the front-end compiler). The binary modules can be quickly deserialized and run by the VM.

The formal grammar for our binary format is as follows:

```
Module := Header *Section
Header := MagicNumber('b' '9' 'm' 'o' 'd' 'u' 'l' 'e') Section
Section := SectionCode SectionBody
SectionCode := FuncionSectionCode(uint32) | StringSectionCode(uint32)
SectionBody := FunctionSectionBody | StringSectionBody
FunctionSectionBody := FunctionCount(uint32) Function
Function := sizeofName (uint32) name(char*) functionIndex(uint32) nargs(uint32) nregs(uint32) *Instruction
Instruction := ByteCode(uint8) Immediate(uint24)
SectionCode:= StringSectionCode(uint32)
StringSectionBody := StringCount(uint32) StringTable
StringTable:= *String
String:= sizeofString(uint32) String(char*)
```

Let's view the above information visually using the diagrams below.

The first diagram depicts the two sections of the binary module: the function section and the string section.

<figure class="image">
  <figcaption>Binary Module Sections</figcaption>
  <img src="{{"/assets/images/binModSections.png" | relative_url }}" width="100%"/>
</figure>

The second diagram depicts the function section of the binary module and the layout of the instructions.

<figure class="image">
  <figcaption>Function Section</figcaption>
  <img src="{{"/assets/images/binModFunctions.png" | relative_url }}" width="100%"/>
</figure>

The last diagram depicts the string section of the binary module, and the layout of the strings within. Because the strings are not null-terminated, the size of each string is stored for the purpose of iteration.

<figure class="image">
  <figcaption>String Section</figcaption>
  <img src="{{"/assets/images/binModStrings.png" | relative_url }}" width="100%"/>
</figure>


### Binary Module Example

```
62 39 6d 6f 64 75 6c 65  01 00 00 00 01 00 00 00  04 00 00 00 66 75 6e 63
00 00 00 00 00 00 00 00  00 00 00 00 01 00 00 0d  02 00 00 0d 00 00 00 09
00 00 00 02 00 00 00 00  02 00 00 00 01 00 00 00  04 00 00 00 63 6f 64 65
```

The above binary module is not translated from any particular JavaScript program, but is meant to serve as the simplest possible example.

It is storing:
- Header "b9module" `62 39 6d 6f 64 75 6c 65` - at the start of every binary module
- Function section code `01 00 00 00` - denotes that the function section is to follow
- Function count `01 00 00 00` - denotes the number of functions in the function section
- Size of function name `04 00 00 00`
- Function name "func" `66 75 6e 63`
- Function index `00 00 00 00`
- Number of arguments `00 00 00 00`
- Number of registers `00 00 00 00`
- Bytecodes:
  - int_push_constant 1 `01 00 00 0d`
  - int_push_constant 2 `02 00 00 0d`
  - int_add `00 00 00 09`
  - function_return `00 00 00 02`
  - end_section `00 00 00 00`
- String section code `02 00 00 00` - denotes that the string section is to follow
- String count `01 00 00 00` - denotes the number of strings in the string section
- Size of string `04 00 00 00`
- String "code" `63 6f 64 65`

Some important things to note:

All strings (or characters) are stored by their hexadecimal [ascii value]. The function section code is always `1` and the string section code is always `2`. The bytecodes are 32-bits wide, with the first three high-order bytes storing the immediate value (if applicable) and the low-order byte storing the bytcode.

[ascii value]: https://www.asciitable.com
