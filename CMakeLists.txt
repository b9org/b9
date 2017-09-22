cmake_minimum_required(VERSION 3.2)

list(APPEND CMAKE_MODULE_PATH
	cmake/Modules
	${CMAKE_CURRENT_LIST_DIR}/omr/cmake/modules/
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

# B9 basic configuration library

add_library(b9interface INTERFACE)

target_include_directories(b9interface
	INTERFACE
		include
		${CMAKE_CURRENT_BINARY_DIR}/include
)

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

# Subdirectories

add_subdirectory(omr)

add_subdirectory(googletest)

add_subdirectory(b9)

add_subdirectory(b9run)

add_subdirectory(b9test)

# b9 programs
add_b9_program(program)
