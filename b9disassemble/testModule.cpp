#include <b9/interpreter.hpp>
#include <b9/loader.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>

#include <gtest/gtest.h>
#include <b9/serialize.hpp>

namespace b9 {
namespace test {

TEST (DisassemblerTest, arguments) {
  b9::VirtualMachine vm {{}};
  auto m = std::make_shared<Module>();
  std::vector<Instruction> i =
    {{ByteCode::INT_PUSH_CONSTANT, 1},
                     {ByteCode::INT_PUSH_CONSTANT, 2},
                     {ByteCode::INT_ADD, 0},
                     {ByteCode::FUNCTION_RETURN, 0},
                     END_SECTION};
  m->functions.push_back(b9::FunctionSpec{"simple_add", i, 2, 2});
  //parseModule(m);
  
  
  vm.load(m);
  auto r = vm.run("simple_add", {1, 2});
  EXPECT_EQ(r, 3);
}
  
}  // namespace test
}  // namespace b9

