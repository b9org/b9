# OMR Configuration
set(OMR_GC ON CACHE INTERNAL "Enable the GC")
set(OMR_GC_GLUE_TARGET   "omr_om_gc_glue"   CACHE STRING "The gc glue target, must be interface library")
set(OMR_UTIL_GLUE_TARGET "omr_om_util_glue" CACHE STRING "The util glue target, must be interface library")
set(OMR_RAS_GLUE_TARGET  "omr_om_ras_glue"  CACHE STRING "The ras glue target, must be interface library")
set(OMR_CORE_GLUE_TARGET "omr_om_core_glue" CACHE STRING "The core glue target, must be and interface library")

configure_file(
	include/OMR/Om/Config.hpp.in
	include/OMR/Om/Config.hpp
)

add_library(omr_om INTERFACE)

target_include_directories(omr_om
	INTERFACE
		include/
		${CMAKE_CURRENT_BINARY_DIR}/include
		../third_party/omr/include_core/
		${CMAKE_BINARY_DIR}/third_party/omr/
)

target_link_libraries(omr_om
	INTERFACE
		omrgc
		omrcore
)

target_compile_definitions(omr_om
	INTERFACE
		-DUT_DIRECT_TRACE_REGISTRATION
)

add_subdirectory(glue)
add_subdirectory(test)
