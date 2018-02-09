add_library(b9 SHARED
	src/core.cpp
	src/loader.cpp
	src/jit.cpp
	src/primitives.cpp
	src/ExecutionContext.cpp
)

target_include_directories(b9
	PUBLIC
		include/
)

target_link_libraries(b9
	PUBLIC
		dl
		jitbuilder
		omr_om
)
