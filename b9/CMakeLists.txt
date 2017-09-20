add_library(b9 SHARED
	core.cpp
	hash.cpp
	jit.cpp
)

target_link_libraries(b9
	PUBLIC
		b9interface
		dl
		jitbuilder
)
