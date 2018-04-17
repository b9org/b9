add_executable (b9disasm
  b9disasm.cpp 
)

target_link_libraries(b9disasm
  PUBLIC
    b9 
)

add_executable(b9serializeTest
  testDisasm.cpp
)

target_link_libraries(b9serializeTest
  PUBLIC
    b9
    gtest_main
)

add_test(
  NAME run_b9serializeTest
  COMMAND b9serializeTest
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

function(b9disasm_test module)
  add_test(
    NAME disasm_${module}
    COMMAND b9disasm ${CMAKE_CURRENT_SOURCE_DIR}/${module}
  ) 
endfunction()

# This test material is missing from the repository
#b9disasm_test(empty.mod)
#b9disasm_test(simple.mod)
#b9disasm_test(corrupt.mod)
#b9disasm_test(complex.mod)
