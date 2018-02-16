#if !defined(B9_EXECUTIONCONTEXT_HPP_)
#define B9_EXECUTIONCONTEXT_HPP_

#include <b9/OperandStack.hpp>
#include <b9/VirtualMachine.hpp>

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
  void visit(Om::Context &cx, VisitorT &visitor) {
    stack_.visit(cx, visitor);
  }

  Om::RunContext &omContext() { return omContext_; }

  operator Om::RunContext &() { return omContext_; }

  operator const Om::RunContext &() const { return omContext_; }

  VirtualMachine *virtualMachine() const { return virtualMachine_; }

  // Available externally for jit-to-primitive calls.
  void doPrimitiveCall(Parameter value);

  friend std::ostream &operator<<(std::ostream &stream,
                                  const ExecutionContext &ec);

 private:
  friend class VirtualMachine;
  friend class ExecutionContextOffset;

  void doFunctionCall(Parameter value);

  void doFunctionReturn(StackElement returnVal);

  Parameter doJmp(Parameter offset);

  void doDuplicate();

  void doDrop();

  void doPushFromVar(StackElement *args, Parameter offset);

  void doPushIntoVar(StackElement *args, Parameter offset);

  void doIntAdd();

  void doIntSub();

  void doIntPushConstant(Parameter value);

  void doIntNot();

  Parameter doIntJmpEq(Parameter delta);

  Parameter doIntJmpNeq(Parameter delta);

  Parameter doIntJmpGt(Parameter delta);

  Parameter doIntJmpGe(Parameter delta);

  Parameter doIntJmpLt(Parameter delta);

  Parameter doIntJmpLe(Parameter delta);

  void doStrPushConstant(Parameter value);

  void doNewObject();

  void doPushFromObject(OMR::Om::Id slotId);

  void doPopIntoObject(OMR::Om::Id slotId);

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

/*
    000
   00000
    000 MMM
       MMMMM
        MMM
*/