add_library(b9 SHARED
	core.cpp
	loader.cpp
	hash.cpp
	jit.cpp
)

target_link_libraries(b9
	PUBLIC
		b9interface
		dl
		jitbuilder
)
