cmake_minimum_required(VERSION 3.2)

find_program(NODE_EXECUTABLE
	NAMES node nodejs
)

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

# OMR Configuration

set(OMR_COMPILER   ON  CACHE INTERNAL "Enable the Compiler.")
set(OMR_JITBUILDER ON  CACHE INTERNAL "We use OMR's jitbuilder tool for the b9 JIT")
set(OMR_GC         ON CACHE INTERNAL "We don't use the GC in b9 (yet)")
set(OMR_FVTEST     OFF CACHE INTERNAL "Disable OMR's internal test suite, it's incompatible with b9")
set(OMR_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "OMR doesn't compile cleanly on my laptop :p")

# Compile a b9-js *.src to C++
function(add_b9_src src)
	add_custom_command(
		OUTPUT
			"${src}.cpp"
		COMMAND
			${NODE_EXECUTABLE} "${CMAKE_SOURCE_DIR}/js_compiler/compile.js"
			"${CMAKE_CURRENT_SOURCE_DIR}/${src}.src"
			>"${src}.cpp"
		DEPENDS
			"${CMAKE_SOURCE_DIR}/js_compiler/compile.js"
			"${CMAKE_CURRENT_SOURCE_DIR}/${src}.src"
	)
endfunction(add_b9_src)

# Compile a b9-js *.src file to a module DLL
function(add_b9_module module)
	add_b9_src(${module})
	add_library(${module} SHARED "${module}.cpp")
	set_target_properties(${module}
    		PROPERTIES
      		PREFIX ""
      		DEBUG_POSTFIX ""
      		SUFFIX ".b9mod"
  	)
  	target_link_libraries("${module}" b9)
endfunction(add_b9_module)

# Add a b9 test program written in b9-js
function(add_b9_test test)
	add_b9_module("${test}")
	add_test(
		NAME "run_${test}"
		COMMAND b9run $<TARGET_FILE:${test}>
	)
	add_test(
		NAME "run_${test}_jit"
		COMMAND b9run -jit $<TARGET_FILE:${test}>
	)
	add_test(
		NAME "run_${test}_jit_directcall"
		COMMAND b9run -jit -directcall $<TARGET_FILE:${test}>
	)
	add_test(
		NAME "run_${test}_jit_passparam"
		COMMAND b9run -jit -directcall -passparam $<TARGET_FILE:${test}>
	)
	add_test(
		NAME "run_${test}_jit_lazyvmstate"
		COMMAND b9run -jit -directcall -passparam -lazyvmstate $<TARGET_FILE:${test}>
	)
endfunction(add_b9_test)

# Subdirectories

add_subdirectory(b9)

add_subdirectory(b9run)

add_subdirectory(b9disassemble)

add_subdirectory(test)

add_subdirectory(third_party)
