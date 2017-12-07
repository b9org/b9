#include <b9/hash.hpp>
#include <b9/interpreter.hpp>

#include <iostream>

using namespace b9;

/// ( number -- 0 )
extern "C" void b9_prim_print_number(ExecutionContext *context) {
  StackElement number = context->pop();
  std::cout << number << std::endl;
  context->push({Value::integer, 0});
}

/// ( string -- 0 )
extern "C" void b9_prim_print_string(ExecutionContext *context) {
  auto value = context->pop();
  assert(value.isInteger());
  auto string = context->virtualMachine()->getString(value.getInteger());
  std::cout << string << std::endl;
  context->push({Value::integer, 0});
}

/// ( -- table )
extern "C" void b9_prim_hash_table_allocate(ExecutionContext *context) {
  // TODO: Result = pHeap p = hashTable_allocate(8);
  context->push(Value().setInteger(0));
}

/// ( value key table -- success )
extern "C" void b9_prim_hash_table_put(ExecutionContext *context) {
  auto v = context->pop().getInteger();
  auto k = context->pop().getInteger();
  auto ht = context->pop().getInteger();
  // TODO: result = hashTable_put(context, (pHeap)ht, (hashTableKey)k,
  // (hashTableKey)v);
  auto result = Value().setInteger(0);
  context->push(result);
}

/// ( key table -- value )
extern "C" void b9_prim_hash_table_get(ExecutionContext *context) {
  StackElement k = context->pop();
  StackElement ht = context->pop();
  // TODO: result: hashTable_get(context, (pHeap)ht, (hashTableKey)k))
  context->push(Value().setInteger(0));
}
