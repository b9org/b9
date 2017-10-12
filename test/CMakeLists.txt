
# Test Programs

add_b9_program(fib)
add_b9_program(hello)
add_b9_program(interpreter_test)


# b9 test - The native test

add_executable(b9test
	b9test.cpp
)

target_link_libraries(b9test
	PUBLIC
		b9 gtest_main
)

add_test(
	NAME run_b9test
	COMMAND env B9_TEST_MODULE=$<TARGET_FILE:interpreter_test> $<TARGET_FILE:b9test>
)


