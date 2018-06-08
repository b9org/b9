#include <b9/ExecutionContext.hpp>
#include <b9/VirtualMachine.hpp>
#include <b9/compiler/Compiler.hpp>

#include <omrgc.h>
#include "Jit.hpp"

#include <OMR/Om/ArrayOperations.hpp>
#include <OMR/Om/ShapeOperations.hpp>
#include <OMR/Om/ObjectOperations.hpp>
#include <OMR/Om/RootRef.hpp>
#include <OMR/Om/Value.hpp>

#include <sys/time.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

namespace b9 {

ExecutionContext::ExecutionContext(VirtualMachine &virtualMachine,
                                   const Config &cfg)
    : omContext_(virtualMachine.memoryManager()),
      virtualMachine_(&virtualMachine),
      cfg_(&cfg) {
  omContext().userMarkingFns().push_back(
      [this](Om::MarkingVisitor &v) { this->visit(v); });
}

void ExecutionContext::reset() {
  stack_.reset();
  programCounter_ = 0;
}

Om::Value ExecutionContext::callJitFunction(JitFunction jitFunction,
                                            std::size_t nargs) {
  if (cfg_->verbose) {
    std::cout << "Int: transition to jit: " << jitFunction << std::endl;
  }

  Om::RawValue result = 0;

  if (cfg_->passParam) {
    switch (nargs) {
      case 0: {
        result = jitFunction(this);
      } break;
      case 1: {
        StackElement p1 = pop();
        result = jitFunction(this, p1.raw());
      } break;
      case 2: {
        StackElement p2 = pop();
        StackElement p1 = pop();
        result = jitFunction(this, p1.raw(), p2.raw());
      } break;
      case 3: {
        StackElement p3 = pop();
        StackElement p2 = pop();
        StackElement p1 = pop();
        result = (*jitFunction)(this, p1.raw(), p2.raw(), p3.raw());
      } break;
      default:
        throw std::runtime_error{"Need to add handlers for more parameters"};
        break;
    }
  } else {
    result = jitFunction(this);
  }

  return Om::Value(Om::AS_RAW, result);
}

StackElement ExecutionContext::interpret(const std::size_t functionIndex) {
  auto function = virtualMachine_->getFunction(functionIndex);
  auto argsCount = function->nargs;
  auto jitFunction = virtualMachine_->getJitAddress(functionIndex);

  if (jitFunction) {
    return callJitFunction(jitFunction, argsCount);
  }

  // interpret the method otherwise
  const Instruction *instructionPointer = function->instructions.data();

  StackElement *args = stack_.top() - function->nargs;
  stack_.pushn(function->nregs);

  while (*instructionPointer != END_SECTION) {
    switch (instructionPointer->opCode()) {
      case OpCode::FUNCTION_CALL:
        doFunctionCall(instructionPointer->immediate());
        break;
      case OpCode::FUNCTION_RETURN: {
        auto result = stack_.pop();
        stack_.restore(args);
        return result;
        break;
      }
      case OpCode::PRIMITIVE_CALL:
        doPrimitiveCall(instructionPointer->immediate());
        break;
      case OpCode::JMP:
        instructionPointer += instructionPointer->immediate();
        break;
      case OpCode::DUPLICATE:
        doDuplicate();
        break;
      case OpCode::DROP:
        doDrop();
        break;
      case OpCode::PUSH_FROM_VAR:
        doPushFromVar(args, instructionPointer->immediate());
        break;
      case OpCode::POP_INTO_VAR:
        doPopIntoVar(args, instructionPointer->immediate());
        break;
      case OpCode::INT_ADD:
        doIntAdd();
        break;
      case OpCode::INT_SUB:
        doIntSub();
        break;
      case OpCode::INT_MUL:
        doIntMul();
        break;
      case OpCode::INT_DIV:
        doIntDiv();
        break;
      case OpCode::INT_PUSH_CONSTANT:
        doIntPushConstant(instructionPointer->immediate());
        break;
      case OpCode::INT_NOT:
        doIntNot();
        break;
      case OpCode::INT_JMP_EQ:
        instructionPointer += doIntJmpEq(instructionPointer->immediate());
        break;
      case OpCode::INT_JMP_NEQ:
        instructionPointer += doIntJmpNeq(instructionPointer->immediate());
        break;
      case OpCode::INT_JMP_GT:
        instructionPointer += doIntJmpGt(instructionPointer->immediate());
        break;
      case OpCode::INT_JMP_GE:
        instructionPointer += doIntJmpGe(instructionPointer->immediate());
        break;
      case OpCode::INT_JMP_LT:
        instructionPointer += doIntJmpLt(instructionPointer->immediate());
        break;
      case OpCode::INT_JMP_LE:
        instructionPointer += doIntJmpLe(instructionPointer->immediate());
        break;
      case OpCode::STR_PUSH_CONSTANT:
        doStrPushConstant(instructionPointer->immediate());
        break;
      case OpCode::STR_JMP_EQ:
        // TODO
        break;
      case OpCode::STR_JMP_NEQ:
        // TODO
        break;
      case OpCode::NEW_OBJECT:
        doNewObject();
        break;
      case OpCode::PUSH_FROM_OBJECT:
        doPushFromObject(Om::Id(instructionPointer->immediate()));
        break;
      case OpCode::POP_INTO_OBJECT:
        doPopIntoObject(Om::Id(instructionPointer->immediate()));
        break;
      case OpCode::CALL_INDIRECT:
        doCallIndirect();
        break;
      case OpCode::SYSTEM_COLLECT:
        doSystemCollect();
        break;
      case OpCode::PUSH_FROM_ARG:
        doPushFromArg(args, instructionPointer->immediate());
        break;
      case OpCode::POP_INTO_ARG:
        doPopIntoArg(args, instructionPointer->immediate());
        break;
      default:
        assert(false);
        break;
    }
    instructionPointer++;
    programCounter_++;
  }
  throw std::runtime_error("Reached end of function");
}

void ExecutionContext::push(StackElement value) { stack_.push(value); }

StackElement ExecutionContext::pop() { return stack_.pop(); }

void ExecutionContext::doFunctionCall(Immediate value) {
  auto f = virtualMachine_->getFunction((std::size_t)value);
  auto result = interpret(value);
  push(result);
}

void ExecutionContext::doFunctionReturn(StackElement returnVal) {
  // TODO
}

void ExecutionContext::doPrimitiveCall(Immediate value) {
  PrimitiveFunction *primitive = virtualMachine_->getPrimitive(value);
  (*primitive)(this);
}

Immediate ExecutionContext::doJmp(Immediate offset) { return offset; }

void ExecutionContext::doDuplicate() {
  push(stack_.peek());
}

void ExecutionContext::doDrop() { stack_.pop(); }

void ExecutionContext::doPushFromVar(StackElement *args, Immediate offset) {
  stack_.push(args[offset]);
}

void ExecutionContext::doPopIntoVar(StackElement *args, Immediate offset) {
  args[offset] = stack_.pop();
}

void ExecutionContext::doPushFromArg(StackElement *args, Immediate offset){
  stack_.push(args[offset]);
}

void ExecutionContext::doPopIntoArg(StackElement *args, Immediate offset){
  args[offset] = stack_.pop();
}

void ExecutionContext::doIntAdd() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left + right});
}

void ExecutionContext::doIntSub() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left - right});
}

void ExecutionContext::doIntMul() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left * right});
}

void ExecutionContext::doIntDiv() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left / right});
}

void ExecutionContext::doIntPushConstant(Immediate value) {
  stack_.push(StackElement().setInt48(value));
}

void ExecutionContext::doIntNot() {
  auto x = stack_.pop().getInt48();
  push({Om::AS_INT48, !x});
}

Immediate ExecutionContext::doIntJmpEq(Immediate delta) {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left == right) {
    return delta;
  }
  return 0;
}

Immediate ExecutionContext::doIntJmpNeq(Immediate delta) {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left != right) {
    return delta;
  }
  return 0;
}

Immediate ExecutionContext::doIntJmpGt(Immediate delta) {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left > right) {
    return delta;
  }
  return 0;
}

// ( left right -- )
Immediate ExecutionContext::doIntJmpGe(Immediate delta) {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left >= right) {
    return delta;
  }
  return 0;
}

// ( left right -- )
Immediate ExecutionContext::doIntJmpLt(Immediate delta) {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left < right) {
    return delta;
  }
  return 0;
}

// ( left right -- )
Immediate ExecutionContext::doIntJmpLe(Immediate delta) {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left <= right) {
    return delta;
  }
  return 0;
}

// ( -- string )
void ExecutionContext::doStrPushConstant(Immediate param) {
  stack_.push({Om::AS_INT48, param});
}

// ( -- object )
void ExecutionContext::doNewObject() {
  auto ref = Om::allocateEmptyObject(*this);
  stack_.push(Om::Value{Om::AS_REF, ref});
}

// ( object -- value )
void ExecutionContext::doPushFromObject(Om::Id slotId) {
  auto value = stack_.pop();
  if (!value.isRef()) {
    throw std::runtime_error("Accessing non-object value as an object.");
  }
  auto obj = value.getRef<Om::Object>();
  Om::SlotDescriptor descriptor;
  auto found = Om::lookupSlot(*this, obj, slotId, descriptor);
  if (found) {
    Om::Value result;
    result = Om::getValue(*this, obj, descriptor);
    stack_.push(result);
  } else {
    throw std::runtime_error("Accessing an object's field that doesn't exist.");
  }
}

// ( object value -- )
void ExecutionContext::doPopIntoObject(Om::Id slotId) {
  if (!stack_.peek().isRef()) {
    throw std::runtime_error("Accessing non-object as an object");
  }

  std::size_t offset = 0;
  auto object = stack_.pop().getRef<Om::Object>();

  Om::SlotDescriptor descriptor;
  bool found = Om::lookupSlot(*this, object, slotId, descriptor);

  if (!found) {
    static constexpr Om::SlotType type(Om::Id(0), Om::CoreType::VALUE);

    Om::RootRef<Om::Object> root(*this, object);
    auto map = Om::transitionLayout(*this, root, {{type, slotId}});
    assert(map != nullptr);

    // TODO: Get the descriptor fast after a single-slot transition.
    Om::lookupSlot(*this, object, slotId, descriptor);
    object = root.get();
  }

  auto val = pop();
  Om::setValue(*this, object, descriptor, val);
  // TODO: Write barrier the object on store.
}

void ExecutionContext::doCallIndirect() {
  assert(0);  // TODO: Implement call indirect
}

void ExecutionContext::doSystemCollect() {
  std::cout << "SYSTEM COLLECT!!!" << std::endl;
  OMR_GC_SystemCollect(omContext_.vmContext(), 0);
}

}  // namespace b9
