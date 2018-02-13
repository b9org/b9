#include <b9/ExecutionContext.hpp>

#include <iostream>

using namespace b9;

/// ( number -- 0 )
extern "C" void b9_prim_print_number(ExecutionContext *context) {
  StackElement number = context->pop();
  std::cout << number << std::endl;
  context->push(OMR::Om::Value(0));
}

/// ( string -- 0 )
extern "C" void b9_prim_print_string(ExecutionContext *context) {
  auto value = context->pop();
  assert(value.isInteger());
  auto string = context->virtualMachine()->getString(value.getInteger());
  std::cout << string << std::endl;
  context->push(OMR::Om::Value(0));
}
