#include <b9/JitHelpers.hpp>

extern "C" {

Om::RawValue interpret0(b9::ExecutionContext *context, std::size_t target) {
  return b9::JitHelper::interpret(*context, target);
}

Om::RawValue interpret1(b9::ExecutionContext *context, std::size_t target,
                        Om::RawValue p0) {
  return b9::JitHelper::interpret(*context, target, p0);
}

Om::RawValue interpret2(b9::ExecutionContext *context, std::size_t target,
                        Om::RawValue p0, Om::RawValue p1) {
  return b9::JitHelper::interpret(*context, target, p0, p1);
}

Om::RawValue interpret3(b9::ExecutionContext *context, std::size_t target,
                        Om::RawValue p0, Om::RawValue p1, Om::RawValue p2) {
  return b9::JitHelper::interpret(*context, target, p0, p1, p2);
}

void primitive_call(b9::ExecutionContext *context, b9::Immediate target) {
  PrimitiveFunction *primitive =
      context->virtualMachine()->getPrimitive(target);
  (*primitive)(context);
}

void trace(const b9::FunctionDef *function, const b9::Instruction *ip) {
  b9::printTrace(std::cerr, *function, ip) << std::endl;
}

void backtrace(const b9::ExecutionContext* context) {
  context->backtrace(std::cerr);
}

void print_stack(const b9::ExecutionContext *context) {
  b9::printStack(std::cerr, context->stack());
}

}  // extern "C"
