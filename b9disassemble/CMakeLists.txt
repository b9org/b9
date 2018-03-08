add_executable (b9disassemble
  b9disassemble.cpp 
)

target_link_libraries(b9disassemble
  PUBLIC
    b9 
)

add_executable(b9serializeTest
  testModule.cpp
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

function(b9disassemble_test module)
  add_test(
    NAME disassemble_${module}
    COMMAND b9disassemble ${CMAKE_CURRENT_SOURCE_DIR}/${module}
  ) 
endfunction()

# This test material is missing from the repository
#b9disassemble_test(empty.mod)
#b9disassemble_test(simple.mod)
#b9disassemble_test(corrupt.mod)
#b9disassemble_test(complex.mod)
