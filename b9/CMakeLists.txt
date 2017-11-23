add_library(b9 SHARED
	core.cpp
	loader.cpp
	hash.cpp
	jit.cpp
	primitives.cpp
)

target_include_directories(b9
	PUBLIC
		include/
		../third_party/omr/include_core/
		${CMAKE_BINARY_DIR}/third_party/omr/
)

target_compile_definitions(b9 PUBLIC -DUT_DIRECT_TRACE_REGISTRATION)

target_link_libraries(b9
	PUBLIC
		dl
		jitbuilder
		omrgc
		omrcore
		b9objects
)
