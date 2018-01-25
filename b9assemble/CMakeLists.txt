add_executable (b9assemble
  b9assemble.cpp
)

target_link_libraries(b9assemble
  PUBLIC
    b9
)

add_executable(b9assembleTest
  testAssemble.cpp
)

target_link_libraries(b9assembleTest
  PUBLIC
    b9
    gtest_main
)

add_test(
  NAME run_assembleTest
  COMMAND b9assembleTest
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

