#if !defined(B9_CALLSTYLE_HPP_)
#define B9_CALLSTYLE_HPP_

#include <ostream>

namespace b9 {

/// The VM callstyle.
/// The b9 VM JIT can generate function calls in a variety of ways.
/// The most basic way, interpreter, has the JIT emit calls back into the interpreter, which performs the call.
/// Then there is direct call, where the JIT will emit direct C-style calls into other functions. However, the code
/// still follows the interpreter's calling convention. Most importantly, function parameters are still placed on the
/// operand stack.
enum class CallStyle { interpreter, direct, passParameter, operandStack };

/// Convert a callstyle enum to a string.
inline const char* toString(const CallStyle callStyle) {
    const char* x = "";
    switch(callStyle) {
    case CallStyle::interpreter:   x = "interpreter";        break;
    case CallStyle::direct:        x = "direct";             break;
    case CallStyle::passParameter: x = "passParameter";      break;
    case CallStyle::operandStack:  x = "operandStack";       break;
    default:                       x = "unknown convention"; break;
    }
    return x;
}

/// Print a callstyle.
inline std::ostream& operator<<(std::ostream& out, const CallStyle style) {
    return out << toString(style);
}

} // namespace b9

#endif // B9_CALLSTYLE_HPP_
