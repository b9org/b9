# Om Debug Options

set(OMR_OM_POISON_RECLAIMED_MEMORY OFF CACHE BOOL "Debug: Posion reclaimed memory")
set(OMR_OM_COLLECT_ON_ALLOCATE OFF CACHE BOOL "Debug: do a full system collect on every allocation")
set(OMR_OM_TRACE OFF CACHE BOOL "Debug: Verbose tracing prints to stderr")

# OMR Configuration
set(OMR_GC ON CACHE INTERNAL "Enable the GC")
set(OMR_GC_GLUE_TARGET   "omr_om_gc_glue"   CACHE INTERNAL "The gc glue target, must be interface library")
set(OMR_UTIL_GLUE_TARGET "omr_om_util_glue" CACHE INTERNAL "The util glue target, must be interface library")
set(OMR_RAS_GLUE_TARGET  "omr_om_ras_glue"  CACHE INTERNAL "The ras glue target, must be interface library")
set(OMR_CORE_GLUE_TARGET "omr_om_core_glue" CACHE INTERNAL "The core glue target, must be and interface library")
set(OMR_TRACE_ROOT "${CMAKE_CURRENT_SRC_DIR}")

configure_file(
	include/OMR/Om/Config.hpp.in
	include/OMR/Om/Config.hpp
)

add_library(omr_om
	src/MemoryManager.cpp
)

set_property(TARGET omr_om PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(omr_om
	PUBLIC
		include/
		${CMAKE_CURRENT_BINARY_DIR}/include
		../third_party/omr/include_core/
		${CMAKE_BINARY_DIR}/third_party/omr/
)

target_link_libraries(omr_om
	PUBLIC
		omrgc
		omrcore
		omrutil
)

target_compile_definitions(omr_om
	PUBLIC
		-DUT_DIRECT_TRACE_REGISTRATION
)

target_compile_options(omr_om
	PRIVATE
		-Wall
)

add_subdirectory(glue)
add_subdirectory(test)
