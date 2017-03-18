cmake_minimum_required(VERSION 3.2)

project(base9
    LANGUAGES CXX
    VERSION 0.1)

enable_testing()

ExternProject_Add(Omr
    PREFIX omr
    CONFIGURE_COMMAND ./configure)

ExternalProject_Add(Base9 DEPENDS Omr)

# Add additional cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# import libjitbuilder
find_package(Omr REQUIRED COMPONENTS libjitbuilder)

#libb9
add_library(libb9 SHARED
    b9.cpp
    b9hash.cpp
    b9jit.cpp)

add_definitions(${libjitbuilder_DEFINITIONS})
include_directories(${libjitbuilder_INCLUDE_DIRS})
target_link_libraries(libb9 dl ${libjitbuilder_LIBRARIES})

# b9
add_executable(b9
    main.cpp)
target_link_libraries(b9 libb9)

# b9test
add_executable(b9test
    b9test.cpp)
target_link_libraries(b9test libb9)
add_test(NAME b9test COMMAND b9test)

