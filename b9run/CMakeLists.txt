add_executable(b9run
	main.cpp
)

target_link_libraries(b9run b9)

add_custom_command(TARGET b9run POST_BUILD
	COMMAND ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "./" $<TARGET_FILE:b9run>
)

