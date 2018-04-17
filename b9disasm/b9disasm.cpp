#include <iostream>
#include <fstream>

#include <b9/deserialize.hpp>
#include <b9/module.hpp>

using namespace b9;

extern "C" int main(int argc, char** argv) {
  std::ifstream infile;
  std::streambuf* inbuffer = nullptr;

  if (argc == 1) {
    inbuffer = std::cin.rdbuf();
  }
  else {
    infile.open(argv[1], std::ios::in | std::ios::binary);
    inbuffer = infile.rdbuf();
  }

  std::istream in(inbuffer);

  auto module = deserialize(in);
  std::cout << *module;
}
