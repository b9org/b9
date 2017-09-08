cmake_minimum_required(VERSION 3.2)

project(base9
    LANGUAGES CXX
    VERSION 0.1)

set (CMAKE_CXX_STANDARD 11)

# Create the compile commands database for clang based tooling
set (EXPORT_COMPILE_COMMANDS 1)
set (CMAKE_CXX_COMPILER clang++)

enable_testing()

# Add additional cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(Omr COMPONENTS REQUIRED JitBuilder)

# libb9
add_library(b9 SHARED
    base9.hpp
    b9core.cpp
    b9hash.cpp
    )
#b9jit.cpp

target_include_directories(b9 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(b9 dl Omr::JitBuilder)

# b9
add_executable(base9
    main.cpp)
target_link_libraries(base9 b9)

# b9test
#add_executable(b9test
#    b9test.cpp)
#target_link_libraries(b9test libb9)
#add_test(NAME b9test COMMAND b9test)

# b9 programs
add_custom_target(programs)

function(add_b9_program program)
    add_custom_command(
        OUTPUT "${program}.cpp"
        COMMAND node "${CMAKE_SOURCE_DIR}/b9.js"
            "${CMAKE_CURRENT_SOURCE_DIR}/${program}.src"
            >"${program}.cpp"
        DEPENDS "${CMAKE_SOURCE_DIR}/b9.js"
    )
    add_library("${program}" SHARED "${program}.cpp")
    target_link_libraries("${program}" b9)
    add_dependencies(programs "${program}")
endfunction(add_b9_program)

add_b9_program(program)

