add_executable (b9asm
  b9asm.cpp
)

target_link_libraries(b9asm
  PUBLIC
    b9
)

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

