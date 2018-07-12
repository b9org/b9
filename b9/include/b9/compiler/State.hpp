#if !defined(B9_STATE_HPP_)
#define B9_STATE_HPP_

#include <ilgen/BytecodeBuilder.hpp>
#include <ilgen/IlBuilder.hpp>
#include <ilgen/MethodBuilder.hpp>
#include <ilgen/VirtualMachineOperandStack.hpp>
#include <ilgen/VirtualMachineRegister.hpp>
#include <ilgen/VirtualMachineRegisterInStruct.hpp>
#include <ilgen/VirtualMachineState.hpp>

namespace b9 {

/// An interface to working with the state of the B9 execution context.
class State : public TR::VirtualMachineState {
 public:
  /// Push an Om::Value onto the stack
  virtual void pushValue(TR::IlBuilder *, TR::IlValue *value) = 0;

  /// Pop an Om::Value from the stack
  virtual TR::IlValue *popValue(TR::IlBuilder *) = 0;

  /// The size of the stack has been changed as part of an external operation.
  /// Adjust the model to note the new size.
  virtual void adjust(TR::IlBuilder *b, int n) = 0;
};

template <typename BuilderT>
State *state(BuilderT *b) {
  return dynamic_cast<State *>(b->vmState());
}

/// State object that keeps VM structures up to date. Commit and reload are
/// no-ops, since state is always written out immediately. Copy and Merge are
/// also no-ops, there is no model to merge or copy.
class ActiveState : public State {
 public:
  ActiveState(TR::MethodBuilder *b, const GlobalTypes &types) : types_(types) {}

  ActiveState(const ActiveState &) = default;

  /// Push a Value onto the OperandStack. Will actually update the stack.
  virtual void pushValue(TR::IlBuilder *b, TR::IlValue *value) override final {
    TR::IlValue *stack = this->stack(b);

    TR::IlValue *stackTop = b->LoadIndirect("b9::OperandStack", "top_", stack);

    b->StoreAt(stackTop, b->ConvertTo(types_.stackElement, value));

    TR::IlValue *newStackTop =
        b->IndexAt(types_.stackElementPtr, stackTop, b->ConstInt32(1));

    b->StoreIndirect("b9::OperandStack", "top_", stack, newStackTop);
  }

  /// Pop a Value from the OperandStack. Will actually update the stack.
  virtual TR::IlValue *popValue(TR::IlBuilder *b) override final {
    auto stack = this->stack(b);

    TR::IlValue *stackTop = b->LoadIndirect("b9::OperandStack", "top_", stack);

    TR::IlValue *newStackTop =
        b->IndexAt(types_.stackElementPtr, stackTop, b->ConstInt32(-1));

    b->StoreIndirect("b9::OperandStack", "top_", stack, newStackTop);

    TR::IlValue *value = b->LoadAt(types_.stackElementPtr, newStackTop);

    return value;
  }

  virtual void adjust(TR::IlBuilder *b, int n) override final {
    assert(n < 1);
    // TODO: No way to increase stack size in model.
  }

  /// @group VirtualMachineState overrides
  /// @{

  virtual void Commit(TR::IlBuilder *b) override final {
    // nothing to commit
  }

  virtual void Reload(TR::IlBuilder *b) override final {
    // nothing to reload
  }

  virtual VirtualMachineState *MakeCopy() override final {
    return new ActiveState(*this);
  }

  virtual void MergeInto(TR::VirtualMachineState *other,
                         TR::IlBuilder *b) override final {
    // nothing to merge
  }

  /// @}

 private:
  TR::IlValue *stack(TR::IlBuilder *b) {
    return b->StructFieldInstanceAddress("b9::ExecutionContext", "stack_",
                                         b->Load("executionContext"));
  }

  const GlobalTypes &types_;
};

/// Lazy VM state that only commits state on demand.
/// Simulates all state of the virtual machine state while compiled code is
/// running. It simulates the stack and the pointer to the top of the stack.
class ModelState : public State {
 public:
  ModelState(TR::MethodBuilder *b, const GlobalTypes &types)
      : stack_(nullptr), stackTop_(nullptr) {
    stackTop_ = new TR::VirtualMachineRegisterInStruct(
        b, "b9::OperandStack", "stack", "top_", "stackTop");

    stack_ = new TR::VirtualMachineOperandStack(b, 64, types.stackElement,
                                                stackTop_, true, 0);
  }

  ModelState(const ModelState &other)
      : stack_(other.stack_), stackTop_(other.stackTop_) {}

  /// Model a push onto the OperandStack. The Value will be saved in an
  /// intermediate IlValue.
  virtual void pushValue(TR::IlBuilder *b, TR::IlValue *value) override final {
    return stack_->Push(b, value);
  }

  /// Model a pop from the OperandStack. The result is an Il Value.
  virtual TR::IlValue *popValue(TR::IlBuilder *b) override final {
    return stack_->Pop(b);
  }

  virtual void adjust(TR::IlBuilder *b, int n) override final {
    assert(n < 1);
    // TODO: No way to increase stack size yet.
    stack_->Drop(b, -int32_t(n));
  }

  /// @group TR::VirtualMachineState overrides
  /// @{

  virtual void Commit(TR::IlBuilder *b) override final {
    stack_->Commit(b);
    stackTop_->Commit(b);
  }

  virtual void Reload(TR::IlBuilder *b) override final {
    stackTop_->Reload(b);
    stack_->Reload(b);
  }

  virtual TR::VirtualMachineState *MakeCopy() override final {
    return new ModelState(*this);
  }

  virtual void MergeInto(TR::VirtualMachineState *other,
                         TR::IlBuilder *b) override final {
    MergeInto(dynamic_cast<ModelState *>(other), b);
  }

  void MergeInto(ModelState *other, TR::IlBuilder *b) {
    stack_->MergeInto(other->stack_, b);
    stackTop_->MergeInto(other->stackTop_, b);
  }

  /// @}

 private:
  TR::VirtualMachineOperandStack *stack_;
  TR::VirtualMachineRegister *stackTop_;
};

}  // namespace b9

#endif  // B9_STATE_HPP_
