add_library(b9 SHARED
	core.cpp
	hash.cpp
	jit.cpp
	primitives.cpp
	serialize.cpp
	deserialize.cpp
  assemble.cpp
)

target_include_directories(b9
	PUBLIC
		include/
)

target_link_libraries(b9
	PUBLIC
		jitbuilder
)
