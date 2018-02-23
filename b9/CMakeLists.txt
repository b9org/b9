add_library(b9 SHARED
	src/ExecutionContext.cpp
	src/MethodBuilder.cpp
	src/core.cpp
	src/Compiler.cpp
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
		jitbuilder
		omr_om
)
