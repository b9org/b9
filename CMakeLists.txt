cmake_minimum_required(VERSION 3.2)

project(base9
	LANGUAGES CXX
	VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

enable_testing()
add_subdirectory(googletest)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# Find OMR

find_package(Omr COMPONENTS REQUIRED JitBuilder)

# b9 lib

add_library(b9 SHARED
	b9/core.cpp
	b9/hash.cpp
	b9/jit.cpp
)

target_include_directories(b9
	PUBLIC
		include/
)

target_link_libraries(b9
	PUBLIC
		dl
		Omr::JitBuilder
)

# b9run exe

add_executable(b9run
	b9run/main.cpp
)

target_link_libraries(b9run b9)

# b9test exe

add_executable(b9test
	b9test/b9test.cpp
)

target_link_libraries(b9test
	PUBLIC
		b9
		gtest_main
)

add_test(b9test b9test)

# b9 programs

add_custom_target(programs)

function(add_b9_program program)
	add_custom_command(
		OUTPUT "${program}.cpp"
		COMMAND node "${CMAKE_SOURCE_DIR}/b9.js"
			"${CMAKE_CURRENT_SOURCE_DIR}/${program}.src"
			>"${program}.cpp"
		DEPENDS "${CMAKE_SOURCE_DIR}/b9.js"
	)
	add_library("${program}" SHARED "${program}.cpp")
	target_link_libraries("${program}" b9)
	add_dependencies(programs "${program}")
endfunction(add_b9_program)

add_b9_program(program)
