cmake_minimum_required(VERSION 3.2)

find_program(NODE_EXECUTABLE
	NAMES node nodejs
)
if(NOT NODE_EXECUTABLE)
	message(SEND_ERROR "Could not find Node executable")
endif()

list(APPEND CMAKE_MODULE_PATH
	cmake/Modules
	${CMAKE_CURRENT_LIST_DIR}/third_party/omr/cmake/modules/
)

project(base9
	LANGUAGES C CXX
	VERSION 0.1
)

include(OmrPlatform)

# Global Configuration

omr_platform_global_setup()

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

enable_testing()

# Optional debug and testing features

set(B9_GCOV OFF CACHE BOOL "Run tests with gcov to gather code coverage data.")

if(B9_GCOV)
	add_compile_options(
		-fprofile-arcs
		-ftest-coverage
		-O0
		-g
	)
	link_libraries(
		-fprofile-arcs
		-ftest-coverage
	)
endif(B9_GCOV)

set(B9_ASAN OFF CACHE BOOL "Build b9 with clang address sanitizer enabled.")

if(B9_ASAN)
	add_compile_options(
		-fsanitize=address -fno-omit-frame-pointer
	)
	link_libraries(
		-fsanitize=address
	)
endif(B9_ASAN)

set(B9_UBSAN OFF CACHE BOOL "Build b9 with clang undefined behaviour sanitizer.")

if(B9_UBSAN)
	add_compile_options(
		-fsanitize=undefined
	)
	link_libraries(
		-fsanitize=undefined
	)
endif(B9_UBSAN)

# OMR Configuration

set(OMR_COMPILER   ON  CACHE INTERNAL "Enable the Compiler.")
set(OMR_JITBUILDER ON  CACHE INTERNAL "We use OMR's jitbuilder tool for the b9 JIT")
set(OMR_GC         ON  CACHE INTERNAL "Enable the GC")
set(OMR_FVTEST     OFF CACHE INTERNAL "Disable OMR's internal test suite, it's incompatible with b9")
set(OMR_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "OMR doesn't compile cleanly on my laptop :p")

# Compile a b9-js *.src to C++
function(add_b9_module src)
	add_custom_target(compile_${src} ALL
		COMMAND
			${NODE_EXECUTABLE}
			"${CMAKE_SOURCE_DIR}/js_compiler/compile.js"
			"${CMAKE_CURRENT_SOURCE_DIR}/${src}.src"
			"${src}.b9mod"
		DEPENDS
			"${CMAKE_CURRENT_SOURCE_DIR}/${src}.src"
			"${CMAKE_SOURCE_DIR}/js_compiler/compile.js"
	)
endfunction(add_b9_module)

# Add a b9 test program written in b9-js
function(add_b9_test test)
	add_b9_module("${test}")
	add_test(
		NAME "run_${test}"
		COMMAND b9run ${test}.b9mod
	)
	# add_dependencies(run_${test} ${test}.b9mod)
	add_test(
		NAME "run_${test}_jit"
		COMMAND b9run -jit ${test}.b9mod
	)
	add_test(
		NAME "run_${test}_jit_directcall"
		COMMAND b9run -jit -directcall ${test}.b9mod
	)
	add_test(
		NAME "run_${test}_jit_passparam"
		COMMAND b9run -jit -directcall -passparam ${test}.b9mod
	)
	add_test(
		NAME "run_${test}_jit_lazyvmstate"
		COMMAND b9run -jit -directcall -passparam -lazyvmstate ${test}.b9mod
	)
endfunction(add_b9_test)

# Subdirectories

add_subdirectory(b9)

add_subdirectory(b9run)

add_subdirectory(b9disassemble)

add_subdirectory(b9assemble)

add_subdirectory(om)

add_subdirectory(test)

add_subdirectory(third_party)
