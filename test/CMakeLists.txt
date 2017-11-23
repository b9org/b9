
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
		b9 gtest_main b9objects
)

add_test(
	NAME run_b9test
	COMMAND env B9_TEST_MODULE=$<TARGET_FILE:interpreter_test> $<TARGET_FILE:b9test>
)

