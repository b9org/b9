add_executable (b9disassemble
  b9disassemble.cpp
)

function(b9disassemble_test module)
        add_test(diassemble_${module}
                COMMAND b9disassemble ${module}
        )
endfunction()

b9disassemble_test(empty.mod)
b9disassemble_test(valid.mod)
