add_executable(b9run
	main.cpp
)

target_link_libraries(b9run b9)

set_target_properties(b9run
	PROPERTIES
		INSTALL_RPATH .
)

