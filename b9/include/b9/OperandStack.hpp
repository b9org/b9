#if !defined(B9_OPERANDSTACK_HPP_)
#define B9_OPERANDSTACK_HPP_

#include <OMR/Om/Context.hpp>
#include <OMR/Om/Value.hpp>

namespace b9 {

namespace Om = OMR::Om;

using StackElement = Om::Value;

class OperandStack {
 public:
  static constexpr std::size_t SIZE = 1000;

  OperandStack() noexcept : top_(&stack_[0]) {
    memset(stack_, 0, sizeof(stack_));
  }

  void reset() { top_ = &stack_[0]; }

  void push(const StackElement &value) {
    *top_ = value;
    ++top_;
  }

  StackElement *pushn(std::size_t n) {
    top_ += n;
    return top_;
  }

  StackElement pop() {
    --top_;
    return *top_;
  }

  StackElement *popn(std::size_t n) {
    top_ -= n;
    return top_;
  }

  StackElement *top() { return top_; }

  void drop() { --top_; }

  StackElement peek() const { return *(top_ - 1); }

  StackElement *begin() { return stack_; }

  StackElement *end() { return top_; }

  void restore(StackElement *top) { top_ = top; }

  template <typename VisitorT>
  void visit(OMR::Om::Context &cx, VisitorT &visitor) {
    for (StackElement element : *this) {
      if (element.isPtr()) {
        visitor.rootEdge(cx, this, (Om::Cell *)element.getPtr());
      }
    }
  }

 private:
  friend class OperandStackOffset;

  StackElement *top_;
  StackElement stack_[SIZE];
};

inline std::ostream &printStack(std::ostream &out, OperandStack &stack) {
  if (stack.begin() == stack.end()) {
    out << "<stack empty>" << std::endl;
    return out;
  }

  for (auto e : stack) {
    std::cout << e << std::endl;
  }
  return out;
}

struct OperandStackOffset {
  static constexpr std::size_t TOP = offsetof(OperandStack, top_);
  static constexpr std::size_t STACK = offsetof(OperandStack, stack_);
};

}  // namespace b9

#endif  // B9_OPERANDSTACK_HPP_
