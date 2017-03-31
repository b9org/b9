cmake_minimum_required(VERSION 3.2)

project(base9
    LANGUAGES CXX
    VERSION 0.1)

enable_testing()

# Add additional cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(Omr REQUIRED COMPONENTS JitBuilder)

# libb9
add_library(libb9 SHARED
    b9.h
    b9.cpp
    b9hash.cpp
    b9jit.cpp)
target_include_directories(libb9 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(libb9 dl Omr::JitBuilder)

# b9
add_executable(b9
    main.cpp)
target_link_libraries(b9 libb9)

# b9test
add_executable(b9test
    b9test.cpp)
target_link_libraries(b9test libb9)
add_test(NAME b9test COMMAND b9test)

# b9 programs
add_custom_target(programs)

function(add_b9_program program)
    add_custom_command(
        OUTPUT "${program}.cpp"
        COMMAND node "b9.js" "${program}.src" >"${CMAKE_CURRENT_BINARY_DIR}/${program}.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/b9.js"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    add_library("${program}" SHARED "${program}.cpp")
    target_link_libraries("${program}" libb9)
    add_dependencies(programs "${program}")
endfunction(add_b9_program)

add_b9_program(program)

