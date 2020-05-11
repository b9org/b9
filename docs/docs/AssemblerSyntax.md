## Example Program

```lisp
(function "example" 0 0
  (push_string "Hello, World!")
  (call_primitive print_string))
```

## Input

```text
Program        : (WhiteSpace | Expression)*
Expr           : "(" WhiteSpace? ExpressionBody WhiteSpace? ")"
ExprBody       : FunctionExpr | StringExpr
FunctionExpr   : "function" WhiteSpace Str Int Int Operation*
Operation      : "(" OperatorExpr ")"

String "\"" Characters "\""

program := <expression>*
<expression> := "(" ( <function> | <string> ) ")"
<function> := "function" WhiteSpace <string> <int> <int>

<operation> :=
  "(push_string" WHITESPACE <int> )
<whitespace> := " " | "\n" | <comment>

WhiteSpace := 
Comment := ";" 
```

## Operators

### push_string

Syntax:
```lisp
(push_string "string")
```

Push string

###
### call_primitive

(call)
#### Primitives
| 0 | `print_int`    | print an integer
| 1 | `print_string` | print a string
