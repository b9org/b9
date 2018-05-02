
# Test Programs

add_b9_test(factorial)
add_b9_test(fib)
add_b9_test(hello)
add_b9_test(simple_add)

# b9 test - The native test

add_b9_module(interpreter_test)

add_executable(b9test
	b9test.cpp
)

target_link_libraries(b9test
	PUBLIC
		b9 gtest_main
)

add_test(
	NAME run_b9test
	COMMAND env B9_TEST_MODULE=interpreter_test.b9mod $<TARGET_FILE:b9test>
)

# b9 asm test

add_executable(b9asmTest
  testAsm.cpp
)

target_link_libraries(b9asmTest
  PUBLIC
    b9
    gtest_main
)

add_test(
  NAME run_asmTest
  COMMAND b9asmTest
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# b9 disasm test

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
