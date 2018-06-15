#if !defined B9_JITHELPERS_HPP_
#define B9_JITHELPERS_HPP_

#include <b9/ExecutionContext.hpp>

namespace b9 {

struct JitHelper {
  /// Helper for pushing a static number of values onto the operand stack.
  /// usage: push(context, value0, value1, ...);
  template <typename T, typename... Rest>
  static void push(ExecutionContext &context, T &&value, Rest &&... rest) {
    context.stack().push(value);
    push(context, rest...);
  }

  /// Base case for push. Pushes nothing.
  static void push(ExecutionContext &context) {}

  /// Helper for making a call to an interpreted function from a compiled
  /// function. Args are pushed directly as raw values.
  template <typename... Args>
  static Om::RawValue interpret(ExecutionContext &context, std::size_t target,
                                Args &&... args) {
    push(context, Om::Value(Om::AS_RAW, args)...);
    context.enterCall(target);
    context.interpret();
    return context.stack().pop().raw();
  }
};

}  // namespace b9

extern "C" {

/// jit-callable helpers for the jit to call interpreted functions.
/// @group

Om::RawValue interpret0(b9::ExecutionContext *context, std::size_t target);

Om::RawValue interpret1(b9::ExecutionContext *context, std::size_t target,
                        Om::RawValue p1);

Om::RawValue interpret2(b9::ExecutionContext *context, std::size_t target,
                        Om::RawValue p1, Om::RawValue p2);

Om::RawValue interpret3(b9::ExecutionContext *context, std::size_t target,
                        Om::RawValue p1, Om::RawValue p2, Om::RawValue p3);

/// @}

/// jit-callable helper for making primitive calls.
void primitive_call(b9::ExecutionContext *context, b9::Immediate target);

/// jit-callable helper for printing instruction execution traces to stderr.
void trace(const b9::FunctionDef *function, const b9::Instruction *instruction);

/// jit-callable helper for printing the stack to stderr.
void print_stack(const b9::ExecutionContext *context);

}  // extern "C"

#endif  // B9_JITHELPERS_HPP_
