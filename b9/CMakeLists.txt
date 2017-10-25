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
)

target_link_libraries(b9
	PUBLIC
		dl
		jitbuilder
)
