add_library(b9 SHARED
	src/ExecutionContext.cpp
	src/MethodBuilder.cpp
	src/core.cpp
	src/Compiler.cpp
	src/loader.cpp
	src/primitives.cpp
	src/serialize.cpp
	src/deserialize.cpp
	src/assemble.cpp
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
