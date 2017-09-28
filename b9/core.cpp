#include <b9.hpp>

#include <b9/core.hpp>
#include <b9/hash.hpp>

#include "Jit.hpp"

#include <dlfcn.h>
#include <sys/time.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace b9 {

void ExecutionContext::push(StackElement value) {
  *(stackPointer_++) = value;
}

StackElement ExecutionContext::pop() { return *(--stackPointer_); }

void ExecutionContext::functionCall(Parameter value) {
  Instruction *program = virtualMachine_->getFunction(value);
  StackElement result = interpret(program);
  push(result);
}

void ExecutionContext::primitiveCall(Parameter value) {
  PrimitiveFunction *primitive = virtualMachine_->getPrimitive(value);
  (*primitive)(virtualMachine_);
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
  push((StackElement) virtualMachine_->getString(value));
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

#if defined(B9JIT)
  if (!initializeJit()) {
    return false;
  }
#endif  // defined(B9JIT)

  return true;
}

bool VirtualMachine::shutdown() {

#if defined(B9JIT)
  shutdownJit();
#endif  // defined(B9JIT)

  return true;
}

bool VirtualMachine::loadLibrary()
{
  return loadLibrary(name_);
}

bool VirtualMachine::loadLibrary(std::string libraryName)
{
  // we only support loading one library for now
  if (library_ != nullptr) {
    std::cout << "Error loading %s: context already has a library loaded"
              << name_;
    return false;
  }

  char sharelib[128];
  if (verbose_) {
    std::cout << "Loading " << libraryName;
  }
  snprintf(sharelib, sizeof(sharelib), "./%s", libraryName.c_str());

  /* Open the shared object */
  dlerror();
  void *handle = dlopen(sharelib, RTLD_NOW);
  char *error = dlerror();
  if (error) {
    printf("%s\n", error);
    return false;
  }
  library_ = handle;

  /* Get the symbol table */
  struct ExportedFunctionData *table =
      (struct ExportedFunctionData *)dlsym(handle, "b9_exported_functions");
  error = dlerror();
  if (error) {
    printf("%s\n", error);
    return false;
  }
  functions_ = table;

  /* Get the primitive table */
  PrimitiveData *primitives =
      (struct PrimitiveData *)dlsym(handle, "b9_primitives");
  error = dlerror();
  if (error) {
    printf("%s\n", error);
    return false;
  }
  primitives_ = primitives;

  /* Get the string table */
  const char **stringTable =
      (const char **)dlsym(handle, "b9_exported_strings");
  error = dlerror();
  if (error) {
    printf("%s\n", error);
    return false;
  }
  this->stringTable_ = stringTable;

  if (this->debug_ > 0) {
    for (int i = 0; i < functions_->functionCount_; i++) {
      FunctionSpecification *functionSpec = functions_->functionTable_ + i;
      std::cout << "Name: " << functionSpec->name_ << " byteCodes: " << functionSpec->byteCodes_;
    }
  }

  return true;
}

/// ByteCode Interpreter

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

StackElement ExecutionContext::interpret(Instruction *program) {
#if defined(B9JIT)
  uint64_t *address = (uint64_t *)(&program[1]);
  if (*address) {
    StackElement result = 0;
    if (context->passParameters) {
      int argsCount = progArgCount(*program);
      // printf("about to call jit args %d\n", argsCount);
      switch (argsCount) {
        case 0: {
          JIT_0_args jitedcode = (JIT_0_args)*address;
          result = (*jitedcode)();
        } break;
        case 1: {
          JIT_1_args jitedcode = (JIT_1_args)*address;
          StackElement p1 = pop(context);
          result = (*jitedcode)(p1);
        } break;
        case 2: {
          JIT_2_args jitedcode = (JIT_2_args)*address;
          StackElement p2 = pop(context);
          StackElement p1 = pop(context);
          result = (*jitedcode)(p1, p2);
        } break;
        case 3: {
          JIT_3_args jitedcode = (JIT_3_args)*address;
          StackElement p3 = pop(context);
          StackElement p2 = pop(context);
          StackElement p1 = pop(context);
          result = (*jitedcode)(p1, p2, p3);
        } break;
        default:
          printf("Need to add handlers for more parameters\n");
          break;
      }
    } else {
      Interpret jitedcode = (Interpret)*address;
      result = (*jitedcode)(context, program);
    }
    return result;
  }
#endif  // defined(B9JIT)

  int nargs = progArgCount(*program);
  int tmps = progTmpCount(*program);
  // printf("Prog Arg Count %d, tmp count %d\n", nargs, tmps);

  Instruction *instructionPointer = program + 3;
  StackElement *args = stackPointer_ - nargs;
  stackPointer_ += tmps;  // local storage for temps

  while (*instructionPointer != NO_MORE_BYTECODES) {
    // b9PrintStack(context);
    // std::cout << "instruction call " << std::hex << (int) ByteCodes::toByte(Instructions::getByteCode(*instructionPointer)) << std::endl;
    switch (Instructions::getByteCode(*instructionPointer)) {
      case ByteCode::intPushConstant:
        intPushConstant(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::strPushConstant:
        strPushConstant(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::drop:
        drop();
        break;
      case ByteCode::intAdd:
        intAdd();
        break;
      case ByteCode::intSub:
        intSub();
        break;
      case ByteCode::jmp:
        instructionPointer += Instructions::getParameter(*instructionPointer);
        break;
      case ByteCode::intJmpEq:
        instructionPointer +=
            intJmpEq(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::intJmpNeq:
        instructionPointer +=
            intJmpNeq(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::intJmpGt:
        instructionPointer += intJmpGt(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::intJmpGe:
        instructionPointer += intJmpGe(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::intJmpLt:
        instructionPointer += intJmpLt(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::intJmpLe:
        instructionPointer += intJmpLe(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::functionCall:
        functionCall(Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::pushFromVar:
        pushFromVar(args, Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::popIntoVar:
        // TODO bad name, push or pop?
        pushIntoVar(args, Instructions::getParameter(*instructionPointer));
        break;
      case ByteCode::functionReturn: {
        StackElement result = *(stackPointer_ - 1);
        stackPointer_ = args;
        return result;
        break;
      }
      case ByteCode::primitiveCall:
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

#if defined(B9JIT)
uint64_t *getJitAddressSlot(Instruction *p) { return (uint64_t *)&p[1]; }

void setJitAddressSlot(Instruction *p, uint64_t value) {
  uint64_t *slotForJitAddress = getJitAddressSlot(p);
  *getJitAddressSlot(p) = value;
}

bool hasJITAddress(Instruction *p) { return *getJitAddressSlot(p) != 0; }

uint64_t VirtualMachine::getJitAddress(int functionIndex) {
  return functions_[functionIndex].jitAddress;
}

void setJitAddress(ExecutionContext *context, int32_t functionIndex,
                   uint64_t value) {
  context->functions[functionIndex].jitAddress = value;
  setJitAddressSlot(context->functions[functionIndex].program, value);
}

#endif /* defined(B9JIT) */

PrimitiveFunction *VirtualMachine::getPrimitive(uint64_t index) {
  auto primitive = primitives_[index].address;
  if (primitive == nullptr) {
    const char *name = primitives_[index].name;

    *(void **)(&primitive) = dlsym(RTLD_DEFAULT, name);
    const char *error = dlerror();
    if (error) {
      printf("%s\n", error);
    }

    // if (this->debug >= 1) {
    //   printf("cache address %s %p\n", name, primitive);
    // }
    primitives_[index].address = primitive;
  }
  return primitives_[index].address;
}

Instruction *VirtualMachine::getFunction(uint64_t index) {
  return functions_->functionTable_[index].byteCodes_;
}

const char *VirtualMachine::getString(int index) {
  return stringTable_[index];
}

#if defined(B9JIT)
int getFunctionCount(ExecutionContext *context) {
  int functionIndex = 0;
  while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
    functionIndex++;
  }
  return functionIndex;
}

void removeGeneratedCode(ExecutionContext *context, int functionIndex) {
  context->functions[functionIndex].jitAddress = 0;
  setJitAddressSlot(context->functions[functionIndex].program, 0);
}

void removeAllGeneratedCode(ExecutionContext *context) {
  int functionIndex = 0;
  while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
    removeGeneratedCode(context, functionIndex);
    functionIndex++;
  }
}

void generateAllCode(ExecutionContext *context) {
  int functionIndex = 0;
  while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
    generateCode(context, functionIndex);
    functionIndex++;
  }
}

#endif /* defined(B9JIT) */

void ExecutionContext::reset() {
  stackPointer_ = stack_;
  programCounter_ = 0;
}

Instruction *VirtualMachine::getFunctionAddress(const char *functionName) {
  if (library_ == nullptr) {
    printf("Error function %s: context has no library loaded", functionName);
    return nullptr;
  }

  Instruction *function = (Instruction *)dlsym(library_, functionName);
  char *error = dlerror();
  if (error) {
    printf("%s\n", error);
    return nullptr;
  }

  return function;
}

StackElement VirtualMachine::runFunction(Instruction *function) {
  executionContext_.reset();

  /* Push random arguments to send to the program */
  int nargs = progArgCount(*function);
  for (int i = 0; i < nargs; i++) {
    int arg = 100 - (i * 10);
    printf("Pushing args %d: %d\n", i, arg);
    executionContext_.push(arg);
  }

  StackElement result = executionContext_.interpret(function);

  executionContext_.reset();

  return result;
}

StackElement timeFunction(VirtualMachine *virtualMachine, Instruction *function,
                          int loopCount, long *runningTime) {
  struct timeval timeBefore, timeAfter, timeResult;
  StackElement result;
  gettimeofday(&timeBefore, NULL);
  while (loopCount--) {
    result = virtualMachine->runFunction(function);
  }
  gettimeofday(&timeAfter, NULL);
  timersub(&timeAfter, &timeBefore, &timeResult);
  *runningTime = (timeResult.tv_sec * 1000 + (timeResult.tv_usec / 1000));
  return result;
}

//
// Base9 Primitives
//

extern "C" void b9_prim_print_number(ExecutionContext *context) {
  StackElement number = context->pop();
  printf("%ld\n", number);
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
  context->push( (StackElement)hashTable_get(context, (pHeap)ht, (hashTableKey)k));
}

}  // namespace b9
