#include <string.h>
#include <iostream>
#include <memory>
#include <vector>

#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>
#include <b9/module.hpp>
#include <b9/serialize.hpp>

using namespace b9;

extern "C" int main(int argc, char** argv) {
  auto module = deserialize(std::cin);
  std::cout << *module;
}
