add_executable (b9disasm
  b9disasm.cpp
)

target_link_libraries(b9disasm
  PUBLIC
    b9
)
