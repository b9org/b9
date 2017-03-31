#.rst:
# FindOmr
# -------
#
# Finds the Omr Library.
#
# This will define the following variables:
#
# and the following import targets
#

include(FindPackageHandleStandardArgs)

find_package(PkgConfig)

# OMR

set(Omr_FOUND 1)

# JitBuilder

pkg_check_modules(PC_JitBuilder QUIET JitBuilder)

find_path(_jit_dir
    Jit.hpp
    PATHS
      ${CMAKE_SOURCE_DIR}/omr/jitbuilder/release/include
      ${PC_JitBuilder_INCLUDE_DIRS}
)
find_path(_compiler_dir
    ilgen/BytecodeBuilder.hpp
    PATHS
      ${CMAKE_SOURCE_DIR}/omr/jitbuilder/release/include/compiler
      ${PC_JitBuilder_INCLUDE_DIRS}
)
set(Omr_JitBuilder_INCLUDE_DIRS ${_jit_dir} ${_compiler_dir})
mark_as_advanced(_jit_dir _compiler_dir)

find_library(Omr_JitBuilder_LIBRARY
    NAMES jitbuilder
    PATHS
      ${PC_JitBuilder_LIBRARY_DIRS}
      ${CMAKE_SOURCE_DIR}/omr/jitbuilder/release/
)
set(Omr_JitBuilder_LIBRARIES ${Omr_JitBuilder_LIBRARY})
mark_as_advanced(Omr_JitBuilder_LIBRARY)


if(Omr_JitBuilder_INCLUDE_DIRS AND Omr_JitBuilder_LIBRARIES)
    set(Omr_JitBuilder_FOUND 1)
endif()


if(Omr_JitBuilder_FOUND AND NOT TARGET Omr::JitBuilder)
    add_library(Omr::JitBuilder STATIC IMPORTED)
    set_target_properties(Omr::JitBuilder
        PROPERTIES
            IMPORTED_LOCATION "${Omr_JitBuilder_LIBRARY}"
          INTERFACE_COMPILE_OPTIONS "-fno-rtti"
          INTERFACE_INCLUDE_DIRECTORIES "${Omr_JitBuilder_INCLUDE_DIRS}"
    )
endif()

find_package_handle_standard_args(Omr
    FOUND_VAR Omr_FOUND
    REQUIRED_VARS
        Omr_FOUND
    VERSION_VAR Omr_VERSION
    HANDLE_COMPONENTS
)

