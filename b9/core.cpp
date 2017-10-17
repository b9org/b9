#include <b9.hpp>

#include <b9/core.hpp>
#include <b9/hash.hpp>
#include <b9/loader.hpp>

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
  auto result = interpret(f);
  push(result);
}

void ExecutionContext::primitiveCall(Parameter value) {
  PrimitiveFunction *primitive = virtualMachine_->getPrimitive(value);
  (*primitive)(this);
}

void ExecutionContext::pushFromVar(StackElement *args, Parameter offset) {
  push(args[offset]);
}

void ExecutionContext::pushIntoVar(StackElement *args, Parameter offset) {
  args[offset] = pop();
}

void ExecutionContext::drop() { pop(); }

void ExecutionContext::intPushConstant(Parameter value) { push(value); }

void ExecutionContext::strPushConstant(Parameter value) {
  push((StackElement)virtualMachine_->getString(value));
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

/// ExecutionContext

bool VirtualMachine::initialize() {
  if (cfg_.jitEnabled) {
    if (!initializeJit()) {
      return false;
    }
    compiler_ = new Compiler(this, cfg_.jitConfig);
  }

  return true;
}

bool VirtualMachine::shutdown() {
  if (cfg_.jitEnabled) {
    delete (compiler_);
    shutdownJit();
  }
  return true;
}

void VirtualMachine::load(std::shared_ptr<const Module> module) {
  module_ = module;
}

/// ByteCode Interpreter

#if 0

StackElement interpret_0(ExecutionContext *context, Instruction *program) {
  return context->interpret(program);
}
StackElement interpret_1(ExecutionContext *context, Instruction *program,
                         StackElement p1) {
  context->push(p1);
  return context->interpret(program);
}
StackElement interpret_2(ExecutionContext *context, Instruction *program,
                         StackElement p1, StackElement p2) {
  context->push(p1);
  context->push(p2);
  return context->interpret(program);
}
StackElement interpret_3(ExecutionContext *context, Instruction *program,
                         StackElement p1, StackElement p2, StackElement p3) {
  context->push(p1);
  context->push(p2);
  context->push(p3);
  return context->interpret(program);
}

#endif  // 0

StackElement ExecutionContext::interpret(const FunctionSpec *function) {
#if 0
  uint64_t *address = (uint64_t *)(&program[1]);
  if (*address) {
    StackElement result = 0;
    if (virtualMachine_->cfg_- {
      int argsCount = progArgCount(*program);
      switch (argsCount) {
        case 0: {
          JIT_0_args jitedcode = (JIT_0_args)*address;
          result = (*jitedcode)();
        } break;
        case 1: {
          JIT_1_args jitedcode = (JIT_1_args)*address;
          StackElement p1 = pop();
          result = (*jitedcode)(p1);
        } break;
        case 2: {
          JIT_2_args jitedcode = (JIT_2_args)*address;
          StackElement p2 = pop();
          StackElement p1 = pop();
          result = (*jitedcode)(p1, p2);
        } break;
        case 3: {
          JIT_3_args jitedcode = (JIT_3_args)*address;
          StackElement p3 = pop();
          StackElement p2 = pop();
          StackElement p1 = pop();
          result = (*jitedcode)(p1, p2, p3);
        } break;
        default:
          printf("Need to add handlers for more parameters\n");
          break;
      }
    } else {
      // Call the Jit'ed function, passing the parameters on the
      // ExecutionContext stack.
      Interpret jitedcode = (Interpret)*address;
      result = (*jitedcode)(this, program);
    }
    return result;
  }
#endif

  // printf("Prog Arg Count %d, tmp count %d\n", nargs, tmps);

  const Instruction *instructionPointer = function->address;
  StackElement *args = stackPointer_ - function->nargs;
  stackPointer_ += function->nregs;

  while (*instructionPointer != NO_MORE_BYTECODES) {
    // b9PrintStack(context);
    // std::cerr << "instruction call " << std::hex << (int)
    // ByteCodes::toByte(Instructions::getByteCode(*instructionPointer)) <<
    // std::endl;
    switch (Instructions::getByteCode(*instructionPointer)) {
      case ByteCode::INT_PUSH_CONSTANT:
        intPushConstant(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::STR_PUSH_CONSTANT:
        strPushConstant(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::DROP:
        drop();
        break;
      case ByteCode::INT_ADD:
        intAdd();
        break;
      case ByteCode::INT_SUB:
        intSub();
        break;
      case ByteCode::JMP:
        instructionPointer += Instructions::getParameter(*instructionPointer);
        break;
      case ByteCode::INT_JMP_EQ:
        instructionPointer +=
            intJmpEq(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::INT_JMP_NEQ:
        instructionPointer +=
            intJmpNeq(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::INT_JMP_GT:
        instructionPointer +=
            intJmpGt(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::INT_JMP_GE:
        instructionPointer +=
            intJmpGe(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::INT_JMP_LT:
        instructionPointer +=
            intJmpLt(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::INT_JMP_LE:
        instructionPointer +=
            intJmpLe(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::FUNCTION_CALL:
        functionCall(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::PUSH_FROM_VAR:
        pushFromVar(args, Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::POP_INTO_VAR:
        // TODO bad name, push or pop?
        pushIntoVar(args, Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::FUNCTION_RETURN: {
        StackElement result = *(stackPointer_ - 1);
        stackPointer_ = args;
        return result;
        break;
      }
      case ByteCode::PRIMITIVE_CALL:
        primitiveCall(Instructions::getParameter(*instructionPointer));
        break;
      default:
        assert(false);
        break;
    }
    instructionPointer++;
  }
  return *(stackPointer_ - 1);
}

uint64_t *getJitAddressSlot(Instruction *p) { return (uint64_t *)&p[1]; }

void setJitAddressSlot(Instruction *p, uint64_t value) {
  uint64_t *slotForJitAddress = getJitAddressSlot(p);
  *getJitAddressSlot(p) = value;
}

void *VirtualMachine::getJitAddress(std::size_t functionIndex) {
  return compiledFunctions_[functionIndex];
}

void VirtualMachine::setJitAddress(std::size_t functionIndex, void *value) {
  compiledFunctions_[functionIndex] = value;
}

PrimitiveFunction *VirtualMachine::getPrimitive(std::size_t index) {
  return module_->primitives[index];
}

const FunctionSpec *VirtualMachine::getFunction(std::size_t index) {
  return &module_->functions[index];
}

const char *VirtualMachine::getString(int index) {
  return module_->strings[index];
}

std::size_t VirtualMachine::getFunctionCount() {
  return module_->functions.size();
}

// void removeGeneratedCode(ExecutionContext *context, int functionIndex) {
//   context->functions[functionIndex].jitAddress = 0;
//   setJitAddressSlot(context->functions[functionIndex].program, 0);
// }

// void removeAllGeneratedCode(ExecutionContext *context) {
//   int functionIndex = 0;
//   while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
//     removeGeneratedCode(context, functionIndex);
//     functionIndex++;
//   }
// }

void VirtualMachine::generateAllCode() {
  std::size_t i = 0;
  for (auto &functionSpec : module_->functions) {
    auto func = compiler_->generateCode(functionSpec);
    compiledFunctions_[i] = func;
  }
}

void ExecutionContext::reset() {
  stackPointer_ = stack_;
  programCounter_ = 0;
}

StackElement VirtualMachine::run(const std::string &name,
                                 const std::vector<StackElement> &usrArgs) {
  return run(module_->findFunction(name), usrArgs);
}

StackElement VirtualMachine::run(const std::size_t functionIndex,
                                 const std::vector<StackElement> &usrArgs) {
  executionContext_.reset();

  auto f = &module_->functions[functionIndex];

  if (f->nargs != usrArgs.size()) {
    std::stringstream ss;
    ss << f->name << " - Got " << usrArgs.size() << " arguments, expected "
       << f->nargs;
    std::string message = ss.str();
    throw BadFunctionCallException{message};
  }

  // get/dispay function name and num arguments
  if (cfg_.verbose)
    std::cout << "function: " << functionIndex << " nargs: " << f->nargs
              << std::endl;

  // push user defined arguments to send to the program
  for (std::size_t i = 0; i < f->nargs; i++) {
    auto idx = f->nargs - i - 1;
    auto arg = usrArgs[idx];
    std::cout << "Pushing arg[" << idx << "] = " << arg << std::endl;
    executionContext_.push(arg);
  }

  StackElement result = executionContext_.interpret(f);

  executionContext_.reset();

  return result;
}

//
// Base9 Primitives
//

extern "C" void b9_prim_print_number(ExecutionContext *context) {
  StackElement number = context->pop();
  std::cout << number;
  context->push(0);
}

extern "C" void b9_prim_print_string(ExecutionContext *context) {
  char *string = (char *)keyToChar(context->pop());
  puts(string);
  context->push(0);
}

extern "C" void b9_prim_hash_table_allocate(ExecutionContext *context) {
  pHeap p = hashTable_allocate(8);
  // if (context->debug >= 1) {
  //   printf("IN hashTableAllocate %p\n", p);
  // }
  context->push((StackElement)p);
}

extern "C" void b9_prim_hash_table_put(ExecutionContext *context) {
  StackElement v = context->pop();
  StackElement k = context->pop();
  StackElement ht = context->pop();
  // if (context->debug >= 1) {
  // printf("IN hashTablePut %p %p(%s) %p(%s) \n", ht, k, k, v, v);
  // }

  context->push((StackElement)hashTable_put(context, (pHeap)ht, (hashTableKey)k,
                                            (hashTableKey)v));
}

extern "C" void b9_prim_hash_table_get(ExecutionContext *context) {
  StackElement k = context->pop();
  StackElement ht = context->pop();
  context->push(
      (StackElement)hashTable_get(context, (pHeap)ht, (hashTableKey)k));
}

}  // namespace b9
