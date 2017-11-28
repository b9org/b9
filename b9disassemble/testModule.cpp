#include "../b9/include/b9/module.hpp"
#include <b9/instructions.hpp>

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

using namespace b9;

/*Module makeModule() {

  auto module = std::make_shared<Module>();
  module->functions.push_back(
    FunctionSpec {
      "example_function", {
        {ByteCode::INT_PUSH_CONSTANT, 0},
        {ByteCode::FUNCTION_RETURN, 0},
        END_SECTION
      }, 
      0, 0,
    });
}

int main (int argc, char** argv) {
  //Module m = makeModule();
  return 0; 
}*/
