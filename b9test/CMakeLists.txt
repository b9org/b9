add_executable(b9test
	b9test.cpp
)

target_link_libraries(b9test
	PUBLIC
		b9
		gtest_main
)

add_test(b9test b9test)

add_b9_program(interpreter_test)

