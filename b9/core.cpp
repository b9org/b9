#include <b9/hash.hpp>
#include <b9/interpreter.hpp>
#include <b9/jit.hpp>

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

void ExecutionContext::push(StackElement value) { *(stackPointer_++) = value; }

StackElement ExecutionContext::pop() { return *(--stackPointer_); }

void ExecutionContext::functionCall(Parameter value) {
  auto f = virtualMachine_->getFunction((std::size_t)value);
  auto result = interpret(value);
  push(result);
}

void ExecutionContext::functionReturn(StackElement returnVal) {
  // TODO
}

void ExecutionContext::primitiveCall(Parameter value) {
  PrimitiveFunction *primitive = virtualMachine_->getPrimitive(value);
  (*primitive)(this);
}

Parameter ExecutionContext::jmp(Parameter offset) { return offset; }

void ExecutionContext::duplicate() {
  // TODO
}

void ExecutionContext::drop() { pop(); }

void ExecutionContext::pushFromVar(StackElement *args, Parameter offset) {
  push(args[offset]);
}

void ExecutionContext::pushIntoVar(StackElement *args, Parameter offset) {
  args[offset] = pop();
}

void ExecutionContext::intAdd() {
  StackElement right = pop();
  StackElement left = pop();
  StackElement result = left + right;
  push(result);
}

void ExecutionContext::intSub() {
  StackElement right = pop();
  StackElement left = pop();
  StackElement result = left - right;
  push(result);
}

// CASCON2017 - Add intMul() and intDiv() here

void ExecutionContext::intPushConstant(Parameter value) { push(value); }

void ExecutionContext::intNot() {
  StackElement i = pop();
  push(!i);
}

Parameter ExecutionContext::intJmpEq(Parameter delta) {
  StackElement right = pop();
  StackElement left = pop();
  if (left == right) {
    return delta;
  }
  return 0;
}

Parameter ExecutionContext::intJmpNeq(Parameter delta) {
  StackElement right = pop();
  StackElement left = pop();
  if (left != right) {
    return delta;
  }
  return 0;
}

Parameter ExecutionContext::intJmpGt(Parameter delta) {
  StackElement right = pop();
  StackElement left = pop();
  if (left > right) {
    return delta;
  }
  return 0;
}

Parameter ExecutionContext::intJmpGe(Parameter delta) {
  StackElement right = pop();
  StackElement left = pop();
  if (left >= right) {
    return delta;
  }
  return 0;
}

Parameter ExecutionContext::intJmpLt(Parameter delta) {
  StackElement right = pop();
  StackElement left = pop();
  if (left < right) {
    return delta;
  }
  return 0;
}

Parameter ExecutionContext::intJmpLe(Parameter delta) {
  StackElement right = pop();
  StackElement left = pop();
  if (left <= right) {
    return delta;
  }
  return 0;
}

void ExecutionContext::strPushConstant(Parameter value) {
  push((StackElement)(virtualMachine_->getString(value)).c_str());
}

/* void strJmpEq(Parameter delta);
  TODO
} */

/* void strJmpNeq(Parameter delta) {
  TODO
} */

/// ExecutionContext

VirtualMachine::VirtualMachine(const Config &cfg)
    : cfg_{cfg}, executionContext_{this, cfg}, compiler_{nullptr} {
  if (cfg_.verbose) std::cout << "VM initializing..." << std::endl;

  if (cfg_.jit) {
    auto ok = initializeJit();
    if (!ok) {
      throw std::runtime_error{"Failed to init JIT"};
    }

    compiler_ = std::make_shared<Compiler>(this, cfg_);
  }
}

VirtualMachine::~VirtualMachine() noexcept {
  if (cfg_.jit) {
    shutdownJit();
  }
}

void VirtualMachine::load(std::shared_ptr<const Module> module) {
  module_ = module;
  compiledFunctions_.reserve(getFunctionCount());
}

/// ByteCode Interpreter

StackElement interpret_0(ExecutionContext *context,
                         const std::size_t functionIndex) {
  return context->interpret(functionIndex);
}
StackElement interpret_1(ExecutionContext *context,
                         const std::size_t functionIndex, StackElement p1) {
  context->push(p1);
  return context->interpret(functionIndex);
}
StackElement interpret_2(ExecutionContext *context,
                         const std::size_t functionIndex, StackElement p1,
                         StackElement p2) {
  context->push(p1);
  context->push(p2);
  return context->interpret(functionIndex);
}
StackElement interpret_3(ExecutionContext *context,
                         const std::size_t functionIndex, StackElement p1,
                         StackElement p2, StackElement p3) {
  context->push(p1);
  context->push(p2);
  context->push(p3);
  return context->interpret(functionIndex);
}

// For primitive calls
void primitive_call(ExecutionContext *context, Parameter value) {
  context->primitiveCall(value);
}

StackElement ExecutionContext::interpret(const std::size_t functionIndex) {
  auto function = virtualMachine_->getFunction(functionIndex);
  auto argsCount = function->nargs;
  auto jitFunction = virtualMachine_->getJitAddress(functionIndex);

  if (jitFunction) {
    if (cfg_.debug) {
      std::cout << "Calling " << function << " jit: " << jitFunction
                << std::endl;
    }
    StackElement result = 0;
    if (cfg_.passParam) {
      switch (argsCount) {
        case 0: {
          result = jitFunction();
        } break;
        case 1: {
          StackElement p1 = pop();
          result = jitFunction(p1);
        } break;
        case 2: {
          StackElement p2 = pop();
          StackElement p1 = pop();
          result = jitFunction(p1, p2);
        } break;
        case 3: {
          StackElement p3 = pop();
          StackElement p2 = pop();
          StackElement p1 = pop();
          result = (*jitFunction)(p1, p2, p3);
        } break;
        default:
          throw std::runtime_error{"Need to add handlers for more parameters"};
          break;
      }
    } else {
      // Call the Jit'ed function, passing the parameters on the
      // ExecutionContext stack.
      if (cfg_.debug) std::cout << "passing parameters on the stack\n";
      result = jitFunction();
    }
    return result;
  }

  // interpret the method otherwise
  const Instruction *instructionPointer = function->instructions.data();
  StackElement *args = stackPointer_ - function->nargs;
  stackPointer_ += function->nregs;

  while (*instructionPointer != END_SECTION) {
    switch (instructionPointer->byteCode()) {
      case ByteCode::FUNCTION_CALL:
        functionCall(instructionPointer->parameter());
        break;
      case ByteCode::FUNCTION_RETURN: {
        StackElement result = *(stackPointer_ - 1);
        stackPointer_ = args;
        return result;
        break;
      }
      case ByteCode::PRIMITIVE_CALL:
        primitiveCall(instructionPointer->parameter());
        break;
      case ByteCode::JMP:
        instructionPointer += instructionPointer->parameter();
        break;
      case ByteCode::DUPLICATE:
        // TODO
        break;
      case ByteCode::DROP:
        drop();
        break;
      case ByteCode::PUSH_FROM_VAR:
        pushFromVar(args, instructionPointer->parameter());
        break;
      case ByteCode::POP_INTO_VAR:
        // TODO bad name, push or pop?
        pushIntoVar(args, instructionPointer->parameter());
        break;
      case ByteCode::INT_ADD:
        intAdd();
        break;
      case ByteCode::INT_SUB:
        intSub();
        break;

        // CASCON2017 - Add INT_MUL and INT_DIV here

      case ByteCode::INT_PUSH_CONSTANT:
        intPushConstant(instructionPointer->parameter());
        break;
      case ByteCode::INT_NOT:
        intNot();
        break;
      case ByteCode::INT_JMP_EQ:
        instructionPointer += intJmpEq(instructionPointer->parameter());
        break;
      case ByteCode::INT_JMP_NEQ:
        instructionPointer += intJmpNeq(instructionPointer->parameter());
        break;
      case ByteCode::INT_JMP_GT:
        instructionPointer += intJmpGt(instructionPointer->parameter());
        break;
      case ByteCode::INT_JMP_GE:
        instructionPointer += intJmpGe(instructionPointer->parameter());
        break;
      case ByteCode::INT_JMP_LT:
        instructionPointer += intJmpLt(instructionPointer->parameter());
        break;
      case ByteCode::INT_JMP_LE:
        instructionPointer += intJmpLe(instructionPointer->parameter());
        break;
      case ByteCode::STR_PUSH_CONSTANT:
        strPushConstant(instructionPointer->parameter());
        break;
      case ByteCode::STR_JMP_EQ:
        // TODO
        break;
      case ByteCode::STR_JMP_NEQ:
        // TODO
        break;
      default:
        assert(false);
        break;
    }
    instructionPointer++;
    programCounter_++;
  }
  return *(stackPointer_ - 1);
}

PrimitiveFunction *const VirtualMachine::primitives_[2];

JitFunction VirtualMachine::getJitAddress(std::size_t functionIndex) {
  if (functionIndex >= compiledFunctions_.size()) {
    return nullptr;
  }
  return compiledFunctions_[functionIndex];
}

void VirtualMachine::setJitAddress(std::size_t functionIndex,
                                   JitFunction value) {
  compiledFunctions_[functionIndex] = value;
}

PrimitiveFunction *VirtualMachine::getPrimitive(std::size_t index) {
  return primitives_[index];
}

const FunctionDef *VirtualMachine::getFunction(std::size_t index) {
  return &module_->functions[index];
}

JitFunction VirtualMachine::generateCode(const std::size_t functionIndex) {
  try {
    return compiler_->generateCode(functionIndex);
  } catch (const CompilationException &e) {
    auto f = getFunction(functionIndex);
    std::cerr << "Warning: Failed to compile " << f << std::endl;
    std::cerr << "    with error: " << e.what() << std::endl;
    return nullptr;
  }
}

std::string VirtualMachine::getString(int index) {
  return module_->strings[index];
}

std::size_t VirtualMachine::getFunctionCount() {
  return module_->functions.size();
}

void VirtualMachine::generateAllCode() {
  assert(cfg_.jit);
  auto functionIndex = 0;

  while (functionIndex < getFunctionCount()) {
    if (cfg_.debug)
      std::cout << "\nJitting function: " << getFunction(functionIndex)->name
                << std::endl;
    auto func = compiler_->generateCode(functionIndex);
    compiledFunctions_.push_back(func);
    ++functionIndex;
  }
}

void ExecutionContext::reset() {
  stackPointer_ = stack_;
  programCounter_ = 0;
}

StackElement VirtualMachine::run(const std::string &name,
                                 const std::vector<StackElement> &usrArgs) {
  return run(module_->getFunctionIndex(name), usrArgs);
}

StackElement VirtualMachine::run(const std::size_t functionIndex,
                                 const std::vector<StackElement> &usrArgs) {
  auto function = getFunction(functionIndex);
  auto argsCount = function->nargs;

  if (cfg_.verbose) {
    std::cout << "+++++++++++++++++++++++" << std::endl;
    std::cout << "Running function: " << function->name
              << " nargs: " << argsCount << std::endl;
  }

  if (argsCount != usrArgs.size()) {
    std::stringstream ss;
    ss << function->name << " - Got " << usrArgs.size()
       << " arguments, expected " << argsCount;
    std::string message = ss.str();
    throw BadFunctionCallException{message};
  }

  // push user defined arguments to send to the program
  for (std::size_t i = 0; i < argsCount; i++) {
    auto idx = argsCount - i - 1;
    auto arg = usrArgs[idx];
    executionContext_.push(arg);
  }

  StackElement result = executionContext_.interpret(functionIndex);

  executionContext_.reset();

  return result;
}

}  // namespace b9
