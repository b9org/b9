#if !defined(B9_EXECUTIONCONTEXT_HPP_)
#define B9_EXECUTIONCONTEXT_HPP_

#include <b9/OperandStack.hpp>
#include <b9/VirtualMachine.hpp>

#include <iostream>

namespace b9 {

// Stack layout in Base9:
//
// clang-format off
//
// |---------- <<< stack upper limit (high address)
// | ...
// |---------- <<< tp
// | Target Scratch Space:
// | * operand 0
// | * operand N
// |---------- <<< bp
// | Stack Frame:
// | * caller type
// | * caller bp
// | * caller ip
// | * caller fn
// |----------
// | Function Locals:
// | * local N
// | * local 0
// |----------
// | Function Parameters (pushed left to right)
// | * param N
// | * param 0
// |---------- <<< arg pointer (bp - (sizeof(frame) + callee.nregs + callee.nargs))
// | Caller Scratch Space:
// | * operand 0
// | * operand N
// | ...
// |---------- <<< stack base (low address)
//
// clang-format on

enum class CallerType {
  INTERPRETER, COMPILED, OUTERMOST
};

class ExecutionContext {
 public:
  ExecutionContext(VirtualMachine &virtualMachine, const Config &cfg);

  StackElement run(std::size_t target, std::vector<StackElement> arguments);

  StackElement run(std::size_t target);

  void reset();

  OperandStack &stack() { return stack_; }

  const OperandStack &stack() const { return stack_; }

  template <typename VisitorT>
  void visit(VisitorT &visitor) {
    stack_.visit(visitor);
  }

  Om::RunContext &omContext() { return omContext_; }

  operator Om::RunContext &() { return omContext_; }

  operator const Om::RunContext &() const { return omContext_; }

  VirtualMachine *virtualMachine() const { return virtualMachine_; }

  StackElement pop() { return stack_.pop(); }

  void push(StackElement e) { return stack_.push(e); }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const ExecutionContext &ec);

  std::ostream &backtrace(std::ostream &out) const;

 protected:
  friend class ExecutionContextOffset;
  friend class JitHelper;

  /// The main interpreter loop.
  void interpret();

  /// A helper for interpreter-to-jit transitions.
  Om::Value callJitFunction(JitFunction jitFunction, std::size_t argCount);

  /// Push all interpreter state to the operand stack. Adjusts bp, but leaves
  /// other state intact. The arguments are already pushed on the stack.
  /// Reserves space for target's locals.
  void enterCall(std::size_t target, CallerType type = CallerType::INTERPRETER);

  /// Pop all interpreter state from the operand stack. Resets all interpreter
  /// state. Pops off the locals and arguments. Does not manage return values.
  void exitCall();

  const FunctionDef &getFunction(std::size_t target) const {
    return virtualMachine()->module()->functions[target];
  }

 private:
  void printCall(std::ostream &out, std::size_t i, std::size_t fn,
                 const Instruction *ip, const Om::Value *tp,
                 const Om::Value *bp) const;

  /// @group Operator Handlers
  /// @{

  void doFunctionCall();

  void doFunctionReturn();

  void doPrimitiveCall();

  void doJmp();

  void doDuplicate();

  void doDrop();

  void doPushFromVar();

  void doPopIntoVar();

  void doIntAdd();

  void doIntSub();

  void doIntMul();

  void doIntDiv();

  void doIntPushConstant();

  void doIntNot();

  void doIntJmpEq();

  void doIntJmpNeq();

  void doIntJmpGt();

  void doIntJmpGe();

  void doIntJmpLt();

  void doIntJmpLe();

  void doStrPushConstant();

  void doNewObject();

  void doPushFromObject();

  void doPopIntoObject();

  void doCallIndirect();

  void doSystemCollect();

  /// @}

  Om::RunContext omContext_;
  OperandStack stack_;
  const Config *cfg_;
  VirtualMachine *virtualMachine_;

  /// @group Interpreter State
  /// @{
  // current function index.
  std::size_t fn_;
  // current instruction pointer.
  const Instruction *ip_;
  // pointer to base of current call frame.
  StackElement *bp_;
  // becomes false when the interpreter must halt.
  bool continue_;
  /// @}
};

static_assert(std::is_standard_layout<ExecutionContext>::value,
              "execution context must be compatible with offsetof");

struct ExecutionContextOffset {
  static constexpr std::size_t OM_CONTEXT =
      offsetof(ExecutionContext, omContext_);

  static constexpr std::size_t STACK = offsetof(ExecutionContext, stack_);

  static constexpr std::size_t IP = offsetof(ExecutionContext, ip_);

  static constexpr std::size_t BP = offsetof(ExecutionContext, bp_);
};

std::ostream &printTrace(std::ostream &out, const b9::FunctionDef &function,
                         const b9::Instruction *ip);

}  // namespace b9

#endif  // B9_EXECUTIONCONTEXT_HPP_
