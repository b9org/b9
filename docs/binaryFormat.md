
# Binary Format Specification

```
module = header *section

section = section_code (function_section / string_section)

section_code = function_section_code / string_section_code

function_section_code = %x01

string_section_code = %x02

function_section = function_count *function

function = name:string, number_of_args:uint32, number_of_registers:uint32, *bytecode end_section

function_name = string

number_of_args = 

string := length:uint32_t char*

end_section = end_section_bytecode

string_section = string_count *string

```