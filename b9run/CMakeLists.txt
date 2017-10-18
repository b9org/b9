add_executable(b9run
	main.cpp
)

set_target_properties(b9run
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(b9run b9)
