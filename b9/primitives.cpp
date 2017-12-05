#include <b9/hash.hpp>
#include <b9/interpreter.hpp>

#include <iostream>

using namespace b9;

/// ( number -- 0 )
extern "C" void b9_prim_print_number(ExecutionContext *context) {
  StackElement number = context->pop();
  std::cout << number << " ";
  context->push(0);
}

/// ( string -- 0 )
extern "C" void b9_prim_print_string(ExecutionContext *context) {
  auto value = context->pop();
  assert(value.isInteger());
  auto string = context->virtualMachine()->getString(value.integer());
  std::cout << string;
  context->push(0);
}

/// ( -- table )
extern "C" void b9_prim_hash_table_allocate(ExecutionContext *context) {
  // TODO: Result = pHeap p = hashTable_allocate(8);
  context->push(Value().integer(0));
}

/// ( value key table -- success )
extern "C" void b9_prim_hash_table_put(ExecutionContext *context) {
  auto v = context->pop().integer();
  auto k = context->pop().integer();
  auto ht = context->pop().integer();
  // TODO: result = hashTable_put(context, (pHeap)ht, (hashTableKey)k,
  // (hashTableKey)v);
  auto result = Value().integer(0);
  context->push(result);
}

/// ( key table -- value )
extern "C" void b9_prim_hash_table_get(ExecutionContext *context) {
  StackElement k = context->pop();
  StackElement ht = context->pop();
  // TODO: result: hashTable_get(context, (pHeap)ht, (hashTableKey)k))
  context->push(Value().integer(0));
}
