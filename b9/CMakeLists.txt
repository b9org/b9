add_library(b9 SHARED
	src/assemble.cpp
	src/Compiler.cpp
	src/deserialize.cpp
	src/ExecutionContext.cpp
	src/JitHelpers.cpp
	src/MethodBuilder.cpp
	src/primitives.cpp
	src/serialize.cpp
	src/VirtualMachine.cpp
)

target_include_directories(b9
	PUBLIC
		include/
)

target_link_libraries(b9
	PUBLIC
		jitbuilder
		omrgc
)
