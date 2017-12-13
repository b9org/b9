#include <iostream>
#include <memory>
#include <vector>
#include <string.h>

#include <b9/module.hpp>
#include <b9/serialize.hpp>
#include <b9/deserialize.hpp>
#include <b9/instructions.hpp>

using namespace b9;

extern "C" int main (int argc, char** argv) {
  auto module = deserialize(std::cin);
}

