#include <b9/ExecutionContext.hpp>

#include <iostream>

using namespace b9;

/// ( number -- 0 )
extern "C" void b9_prim_print_number(ExecutionContext *context) {
  auto number = context->pop();
  assert(number.isInt48());
  std::cout << number << std::endl;
  context->push(Om::Value(Om::AS_INT48, 0));
}

/// ( string -- 0 )
extern "C" void b9_prim_print_string(ExecutionContext *context) {
  auto value = context->pop();
  assert(value.isInt48());
  auto string = context->virtualMachine()->getString(value.getInt48());
  std::cout << string << std::endl;
  context->push({Om::AS_INT48, 0});
}

/// ( -- 0 )
extern "C" void b9_prim_print_stack(ExecutionContext *context) {
  printStack(std::cout, context->stack());
  context->push(Om::Value(Om::AS_INT48, 0));
}
