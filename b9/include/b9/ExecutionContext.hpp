#if !defined(B9_EXECUTIONCONTEXT_HPP_)
#define B9_EXECUTIONCONTEXT_HPP_

#include <b9/OperandStack.hpp>
#include <b9/VirtualMachine.hpp>

#include <iostream>

namespace b9 {

class ExecutionContext {
 public:
  ExecutionContext(VirtualMachine &virtualMachine, const Config &cfg);

  StackElement interpret(std::size_t functionIndex);

  void reset();

  StackElement pop();

  void push(StackElement e);

  const OperandStack &stack() const { return stack_; }

  template <typename VisitorT>
  void visit(VisitorT &visitor) {
    stack_.visit(visitor);
  }

  Om::RunContext &omContext() { return omContext_; }

  operator Om::RunContext &() { return omContext_; }

  operator const Om::RunContext &() const { return omContext_; }

  VirtualMachine *virtualMachine() const { return virtualMachine_; }

  // Available externally for jit-to-primitive calls.
  void doPrimitiveCall(Immediate value);

  friend std::ostream &operator<<(std::ostream &stream,
                                  const ExecutionContext &ec);

 private:
  friend class VirtualMachine;
  friend class ExecutionContextOffset;

  void doFunctionCall(Immediate value);

  /// A helper for interpreter-to-jit transitions.
  Om::Value callJitFunction(JitFunction jitFunction, std::size_t argCount);

  void doFunctionReturn(StackElement returnVal);

  Immediate doJmp(Immediate offset);

  void doDuplicate();

  void doDrop();

  void doPushFromLocal(StackElement *locals, Immediate offset);

  void doPopIntoLocal(StackElement *locals, Immediate offset);

  void doPushFromParam(StackElement *params, Immediate offset);

  void doPopIntoParam(StackElement *params, Immediate offset);

  void doIntAdd();

  void doIntSub();

  void doIntMul();

  void doIntDiv();

  void doIntPushConstant(Immediate value);

  void doIntNot();

  Immediate doJmpEq(Immediate delta);

  Immediate doJmpNeq(Immediate delta);

  Immediate doJmpGt(Immediate delta);

  Immediate doJmpGe(Immediate delta);

  Immediate doJmpLt(Immediate delta);

  Immediate doJmpLe(Immediate delta);

  void doStrPushConstant(Immediate value);

  void doNewObject();

  void doPushFromObject(Om::Id slotId);

  void doPopIntoObject(Om::Id slotId);

  void doCallIndirect();

  void doSystemCollect();

  Om::RunContext omContext_;
  OperandStack stack_;
  const Config *cfg_;
  VirtualMachine *virtualMachine_;
  Instruction *programCounter_ = 0;
};

// static_assert(std::is_standard_layout<ExecutionContext>::value);

struct ExecutionContextOffset {
  static constexpr std::size_t OM_CONTEXT =
      offsetof(ExecutionContext, omContext_);
  static constexpr std::size_t STACK = offsetof(ExecutionContext, stack_);
  static constexpr std::size_t PROGRAM_COUNTER =
      offsetof(ExecutionContext, programCounter_);
};

}  // namespace b9

#endif  // B9_EXECUTIONCONTEXT_HPP_
