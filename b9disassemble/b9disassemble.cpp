#include <iostream>

#include <b9/deserialize.hpp>
#include <b9/module.hpp>

using namespace b9;

extern "C" int main(int argc, char** argv) {
  auto module = deserialize(std::cin);
  std::cout << *module;
}
