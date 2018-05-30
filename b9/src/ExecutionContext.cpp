#include <b9/ExecutionContext.hpp>
#include <b9/VirtualMachine.hpp>
#include <b9/compiler/Compiler.hpp>

#include <omrgc.h>
#include <OMR/Om/Allocator.inl.hpp>
#include <OMR/Om/ArrayBuffer.inl.hpp>
#include <OMR/Om/ArrayBufferMap.inl.hpp>
#include <OMR/Om/Map.inl.hpp>
#include <OMR/Om/Object.inl.hpp>
#include <OMR/Om/ObjectMap.inl.hpp>
#include <OMR/Om/RootRef.inl.hpp>
#include <OMR/Om/Value.hpp>
#include "Jit.hpp"

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
  omContext().userRoots().push_back(
      [this](Om::Context &cx, Om::Visitor &v) { this->visit(cx, v); });
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

  return Om::Value(Om::FROM_RAW, result);
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
        // TODO bad name, push or pop?
        doPushIntoVar(args, instructionPointer->immediate());
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
        doPushFromObject(OMR::Om::Id(instructionPointer->immediate()));
        break;
      case OpCode::POP_INTO_OBJECT:
        doPopIntoObject(OMR::Om::Id(instructionPointer->immediate()));
        break;
      case OpCode::CALL_INDIRECT:
        doCallIndirect();
        break;
      case OpCode::SYSTEM_COLLECT:
        doSystemCollect();
        break;
      default:
        assert(false);
        break;
    }
    instructionPointer++;
    programCounter_++;
  }
  //throw std::runtime_error("Reached end of function");
  return StackElement(0);
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

void ExecutionContext::doPushIntoVar(StackElement *args, Immediate offset) {
  args[offset] = stack_.pop();
}

void ExecutionContext::doIntAdd() {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  StackElement result;
  result.setInteger(left + right);
  push(result);
}

void ExecutionContext::doIntSub() {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  StackElement result;
  result.setInteger(left - right);
  push(result);
}

void ExecutionContext::doIntMul() {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  StackElement result;
  result.setInteger(left * right);
  push(result);
}

void ExecutionContext::doIntDiv() {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  StackElement result;
  result.setInteger(left / right);
  push(result);
}

void ExecutionContext::doIntPushConstant(Immediate value) {
  stack_.push(StackElement().setInteger(value));
}

void ExecutionContext::doIntNot() {
  std::int32_t i = stack_.pop().getInteger();
  StackElement v;
  v.setInteger(!i);
  push(v);
}

Immediate ExecutionContext::doIntJmpEq(Immediate delta) {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  if (left == right) {
    return delta;
  }
  return 0;
}

Immediate ExecutionContext::doIntJmpNeq(Immediate delta) {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  if (left != right) {
    return delta;
  }
  return 0;
}

Immediate ExecutionContext::doIntJmpGt(Immediate delta) {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  if (left > right) {
    return delta;
  }
  return 0;
}

// ( left right -- )
Immediate ExecutionContext::doIntJmpGe(Immediate delta) {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  if (left >= right) {
    return delta;
  }
  return 0;
}

// ( left right -- )
Immediate ExecutionContext::doIntJmpLt(Immediate delta) {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  if (left < right) {
    return delta;
  }
  return 0;
}

// ( left right -- )
Immediate ExecutionContext::doIntJmpLe(Immediate delta) {
  std::int32_t right = stack_.pop().getInteger();
  std::int32_t left = stack_.pop().getInteger();
  if (left <= right) {
    return delta;
  }
  return 0;
}

// ( -- string )
void ExecutionContext::doStrPushConstant(Immediate param) {
  stack_.push(OMR::Om::Value().setInteger(param));
}

// ( -- object )
void ExecutionContext::doNewObject() {
  auto ref = OMR::Om::Object::allocate(*this);
  stack_.push(OMR::Om::Value(ref));
}

// ( object -- value )
void ExecutionContext::doPushFromObject(Om::Id slotId) {
  auto value = stack_.pop();
  if (!value.isPtr()) {
    throw std::runtime_error("Accessing non-object value as an object.");
  }
  auto obj = value.getPtr<Om::Object>();
  Om::SlotDescriptor descriptor;
  auto found = Om::Object::lookup(*this, obj, slotId, descriptor);
  if (found) {
    Om::Value result;
    result = Om::Object::getValue(*this, obj, descriptor);
    stack_.push(result);
  } else {
    throw std::runtime_error("Accessing an object's field that doesn't exist.");
  }
}

// ( object value -- )
void ExecutionContext::doPopIntoObject(Om::Id slotId) {
  if (!stack_.peek().isPtr()) {
    throw std::runtime_error("Accessing non-object as an object");
  }

  std::size_t offset = 0;
  auto object = stack_.pop().getPtr<Om::Object>();

  Om::SlotDescriptor descriptor;
  bool found = Om::Object::lookup(*this, object, slotId, descriptor);

  if (!found) {
    static constexpr Om::SlotType type(Om::Id(0), Om::CoreType::VALUE);

    Om::RootRef<Om::Object> root(*this, object);
    auto map = Om::Object::transition(*this, root, {{type, slotId}});
    assert(map != nullptr);

    // TODO: Get the descriptor fast after a single-slot transition.
    Om::Object::lookup(*this, object, slotId, descriptor);
    object = root.get();
  }

  auto val = pop();
  Om::Object::setValue(*this, object, descriptor, val);
  // TODO: Write barrier the object on store.
}

void ExecutionContext::doCallIndirect() {
  assert(0);  // TODO: Implement call indirect
}

void ExecutionContext::doSystemCollect() {
  std::cout << "SYSTEM COLLECT!!!" << std::endl;
  OMR_GC_SystemCollect(omContext_.omrVmThread(), 0);
}

}  // namespace b9
