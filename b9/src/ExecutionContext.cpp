#include <b9/ExecutionContext.hpp>
#include <b9/JitHelpers.hpp>
#include <b9/VirtualMachine.hpp>
#include <b9/compiler/Compiler.hpp>

#include <omrgc.h>
#include "Jit.hpp"

#include <OMR/Om/ArrayOperations.hpp>
#include <OMR/Om/ObjectOperations.hpp>
#include <OMR/Om/RootRef.hpp>
#include <OMR/Om/ShapeOperations.hpp>
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

ExecutionContext::ExecutionContext(VirtualMachine& virtualMachine,
                                   const Config& cfg)
    : omContext_(virtualMachine.memoryManager()),
      virtualMachine_(&virtualMachine),
      cfg_(&cfg),
      fn_(0),
      ip_(0),
      bp_(stack().top()),
      continue_(true) {
  omContext().userMarkingFns().push_back(
      [this](Om::MarkingVisitor& v) { this->visit(v); });
}

void ExecutionContext::reset() {
  continue_ = true;
  stack_.reset();
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

StackElement ExecutionContext::run(std::size_t target,
                                   std::vector<StackElement> arguments) {
  auto& callee = getFunction(target);
  assert(callee.nargs == arguments.size());

  reset();

  for (auto arg : arguments) {
    stack_.push(arg);
  }

  enterCall(target, CallerType::OUTERMOST);
  interpret();
  return stack_.pop();
}

StackElement ExecutionContext::run(std::size_t target) {
  auto& callee = getFunction(target);
  assert(callee.nargs == 0);

  reset();

  enterCall(target);
  interpret();
  return stack_.pop();
}

void ExecutionContext::interpret() {
  while (continue_) {
    assert(ip_ != 0x00 || ip_ != (Instruction*)0x04);

    if (cfg_->debug) {
      printTrace(std::cerr, getFunction(fn_), ip_) << std::endl;
    }

    switch (ip_->opCode()) {
      case OpCode::FUNCTION_CALL:
        doFunctionCall();
        break;
      case OpCode::FUNCTION_RETURN:
        doFunctionReturn();
        break;
      case OpCode::PRIMITIVE_CALL:
        doPrimitiveCall();
        break;
      case OpCode::JMP:
        doJmp();
        break;
      case OpCode::DUPLICATE:
        doDuplicate();
        break;
      case OpCode::DROP:
        doDrop();
        break;
      case OpCode::PUSH_FROM_VAR:
        doPushFromVar();
        break;
      case OpCode::POP_INTO_VAR:
        doPopIntoVar();
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
        doIntPushConstant();
        break;
      case OpCode::INT_NOT:
        doIntNot();
        break;
      case OpCode::INT_JMP_EQ:
        doIntJmpEq();
        break;
      case OpCode::INT_JMP_NEQ:
        doIntJmpNeq();
        break;
      case OpCode::INT_JMP_GT:
        doIntJmpGt();
        break;
      case OpCode::INT_JMP_GE:
        doIntJmpGe();
        break;
      case OpCode::INT_JMP_LT:
        doIntJmpLt();
        break;
      case OpCode::INT_JMP_LE:
        doIntJmpLe();
        break;
      case OpCode::STR_PUSH_CONSTANT:
        doStrPushConstant();
        break;
      case OpCode::NEW_OBJECT:
        doNewObject();
        break;
      case OpCode::PUSH_FROM_OBJECT:
        doPushFromObject();
        break;
      case OpCode::POP_INTO_OBJECT:
        doPopIntoObject();
        break;
      case OpCode::CALL_INDIRECT:
        doCallIndirect();
        break;
      case OpCode::SYSTEM_COLLECT:
        doSystemCollect();
        break;
      case OpCode::END_SECTION:
        continue_ = false;
        break;
      default:
        assert(false);
        break;
    }

    if (cfg_->debug) {
      backtrace(std::cerr) << std::endl;
      // printStack(std::cerr, stack());
    }
  }
}

std::size_t FRAME_SIZE = sizeof(Om::Value) * 4;

void ExecutionContext::enterCall(std::size_t target, CallerType type) {
  const FunctionDef& callee = getFunction(target);

  // reserve space for locals (args are already pushed)
  stack_.pushn(callee.nregs);

  // save caller's interpreter state.
  stack_.push({Om::AS_UINT48, fn_});
  stack_.push({Om::AS_PTR, ip_});
  stack_.push({Om::AS_PTR, bp_});
  stack_.push({Om::AS_UINT48, std::uint64_t(type)});

  // set up state for callee
  fn_ = target;
  ip_ = callee.instructions.data();
  bp_ = stack_.top();
}

void ExecutionContext::exitCall() {
  const FunctionDef& callee = getFunction(fn_);

  // pop callee scratch space
  stack_.restore(bp_);

  // restore caller state. note IP is restored verbatim, not incremented.
  CallerType type = CallerType(stack_.pop().getUint48());
  bp_ = stack_.pop().getPtr<Om::Value>();
  ip_ = stack_.pop().getPtr<Instruction>();
  fn_ = stack_.pop().getUint48();

  // pop parameters and locals
  stack_.popn(callee.nargs + callee.nregs);

  switch (type) {
    case CallerType::INTERPRETER:
      continue_ = true;
      break;
    case CallerType::OUTERMOST:
      continue_ = false;
      break;
    case CallerType::COMPILED:
      assert(0);
      continue_ = false;
      break;
  }
}

void ExecutionContext::doFunctionCall() { enterCall(ip_->immediate()); }

void ExecutionContext::doFunctionReturn() {
  StackElement result = stack_.pop();
  exitCall();
  stack_.push(result);
  ++ip_;
}

void ExecutionContext::doPrimitiveCall() {
  Immediate index = ip_->immediate();
  PrimitiveFunction* primitive = virtualMachine_->getPrimitive(index);
  (*primitive)(this);
  ++ip_;
}

void ExecutionContext::doJmp() { ip_ += ip_->immediate() + 1; }

void ExecutionContext::doDuplicate() {
  stack_.push(stack_.peek());
  ++ip_;
}

void ExecutionContext::doDrop() {
  stack_.pop();
  ++ip_;
}

void ExecutionContext::doPushFromVar() {
  const FunctionDef& callee = getFunction(fn_);
  Om::Value* args = bp_ - (4 + callee.nargs +
                           callee.nregs);  // TODO: Improve variable indexing
  Immediate index = ip_->immediate();
  stack_.push(args[index]);
  ++ip_;
}

void ExecutionContext::doPopIntoVar() {
  const FunctionDef& callee = getFunction(fn_);
  Om::Value* args = bp_ - (4 + callee.nargs +
                           callee.nregs);  // TODO: Improve variable indexing
  Immediate index = ip_->immediate();
  args[index] = stack_.pop();
  ++ip_;
}

void ExecutionContext::doIntAdd() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left + right});
  ++ip_;
}

void ExecutionContext::doIntSub() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left - right});
  ++ip_;
}

void ExecutionContext::doIntMul() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left * right});
  ++ip_;
}

void ExecutionContext::doIntDiv() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  push({Om::AS_INT48, left / right});
  ++ip_;
}

void ExecutionContext::doIntPushConstant() {
  stack_.push({Om::AS_INT48, ip_->immediate()});
  ++ip_;
}

void ExecutionContext::doIntNot() {
  auto x = stack_.pop().getInt48();
  push({Om::AS_INT48, !x});
  ++ip_;
}

void ExecutionContext::doIntJmpEq() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left == right) {
    ip_ += ip_->immediate() + 1;
  } else {
    ip_++;
  }
}

void ExecutionContext::doIntJmpNeq() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left != right) {
    ip_ += ip_->immediate() + 1;
  } else {
    ++ip_;
  }
}

void ExecutionContext::doIntJmpGt() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left > right) {
    ip_ += ip_->immediate() + 1;
  } else {
    ++ip_;
  }
}

// ( left right -- )
void ExecutionContext::doIntJmpGe() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left >= right) {
    ip_ += ip_->immediate() + 1;
  } else {
    ++ip_;
  }
}

// ( left right -- )
void ExecutionContext::doIntJmpLt() {
  std::int32_t right = stack_.pop().getInt48();
  std::int32_t left = stack_.pop().getInt48();
  if (left < right) {
    ip_ += ip_->immediate() + 1;
  } else {
    ++ip_;
  }
}

// ( left right -- )
void ExecutionContext::doIntJmpLe() {
  auto right = stack_.pop().getInt48();
  auto left = stack_.pop().getInt48();
  if (left <= right) {
    ip_ += ip_->immediate() + 1;
  } else {
    ++ip_;
  }
}

// ( -- string )
void ExecutionContext::doStrPushConstant() {
  stack_.push({Om::AS_INT48, ip_->immediate()});
  ++ip_;
}

// ( -- object )
void ExecutionContext::doNewObject() {
  auto ref = Om::allocateEmptyObject(*this);
  stack_.push({Om::AS_REF, ref});
  ++ip_;
}

// ( object -- value )
void ExecutionContext::doPushFromObject() {
  Om::Id slotId(ip_->immediate());

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
  ++ip_;
}

// ( object value -- )
void ExecutionContext::doPopIntoObject() {
  Om::Id slotId(ip_->immediate());

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
  ++ip_;
}

void ExecutionContext::doCallIndirect() {
  assert(0);  // TODO: Implement call indirect
  ++ip_;
}

void ExecutionContext::doSystemCollect() {
  std::cout << "SYSTEM COLLECT!!!" << std::endl;
  OMR_GC_SystemCollect(omContext_.vmContext(), 0);
  ++ip_;
}

/// prints (top, base]
void printStackSegment(std::ostream& out, const Om::Value* top,
                       const Om::Value* base) {
  out << "\n    (stack";
  for (const Om::Value* x = top - 1; x >= base; --x) {
    out << "\n      " << *x;
  }
  out << ")";
}

void printLocals(std::ostream& out, const Om::Value* locals,
                 std::size_t nlocals) {
  out << "\n    (locals";
  for (std::size_t i = 0; i < nlocals; ++i) {
    out << "\n      "
        << "(local " << i << " " << locals[i] << ")";
  }
  out << ")";
}

void printParams(std::ostream& out, const Om::Value* params,
                 std::size_t nparams) {
  out << "\n    (params";
  for (std::size_t i = 0; i < nparams; ++i) {
    out << "\n      "
        << "(param " << i << " " << params[i] << ")";
  }
  out << ")";
}

std::ostream& ExecutionContext::backtrace(std::ostream& out) const {
  out << "(backtrace";

  // first frame:

  std::size_t fn = fn_;
  const Om::Value* bp = bp_;
  const Instruction* ip = ip_;
  const Om::Value* tp = stack().top();

  std::size_t i = 0;

  while (bp > stack().base()) {
    printCall(out, i, fn, ip, tp, bp);

    // next frame:

    ++i;

    const FunctionDef& callee = getFunction(fn);
    const Om::Value* frame = bp;

    bp = frame[-2].getPtr<Om::Value>();
    ip = frame[-3].getPtr<const Instruction>();
    fn = frame[-4].getUint48();
    tp = frame - 4 - callee.nregs - callee.nargs;
  };

  out << ")";

  return out;
}

void ExecutionContext::printCall(std::ostream& out, std::size_t i,
                                 std::size_t fn, const Instruction* ip,
                                 const Om::Value* tp, const Om::Value* bp) const {
  const FunctionDef& callee = getFunction(fn);
  std::size_t nlocals = callee.nregs;
  std::size_t nparams = callee.nargs;

  out << "\n  (frame " << i;
  out << " (fn " << callee.name << " " << fn << ")";

  printStackSegment(out, tp, bp);
  printLocals(out, bp - 4 - nlocals, nlocals);
  printParams(out, bp - 4 - nlocals - nparams, nparams);

  out << ")";
}

std::ostream& printTrace(std::ostream& out, const b9::FunctionDef& function,
                    const b9::Instruction* ip) {
  return out << "(trace " << function.name << " " << ip << " " << *ip << ")";
}

}  // namespace b9
