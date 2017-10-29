#include <b9/bytecodes.hpp>
#include <b9/interpreter.hpp>
#include <b9/jit.hpp>

#include "Jit.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace b9 {

extern "C" void b9PrintStack(ExecutionContext *context, int64_t pc, int64_t bytecode, int64_t param) {
  printf("Executing at pc %lld, bc is (%d, %s), param is (%d)\n", pc, bytecode, toString((ByteCode)bytecode), param);
	StackElement* base = context->stackBase_;
	std::cout << "------\n";
	while (base < context->stackPointer_) {
		printf("%p: Stack[%ld] = %lld\n", base, base - context->stackBase_, *base);
		base++;
	}
	std::cout << "^^^^^^^^^^^^^^^^^\n";
}

// Simulates all state of the virtual machine state while compiled code is
// running. It simulates the stack and the pointer to the top of the stack.
class VirtualMachineState : public OMR::VirtualMachineState {
 public:
  VirtualMachineState()
      : OMR::VirtualMachineState(), _stack(NULL), _stackTop(NULL) {}

  VirtualMachineState(OMR::VirtualMachineOperandStack *stack,
                      OMR::VirtualMachineRegister *stackTop)
      : OMR::VirtualMachineState(), _stack(stack), _stackTop(stackTop) {}

  virtual void Commit(TR::IlBuilder *b) {
    _stack->Commit(b);
    _stackTop->Commit(b);
  }

  virtual void Reload(TR::IlBuilder *b) {
    _stackTop->Reload(b);
    _stack->Reload(b);
  }

  virtual VirtualMachineState *MakeCopy() {
    VirtualMachineState *newState = new VirtualMachineState();
    newState->_stack = (OMR::VirtualMachineOperandStack *)_stack->MakeCopy();
    newState->_stackTop = (OMR::VirtualMachineRegister *)_stackTop->MakeCopy();
    return newState;
  }

  virtual void MergeInto(VirtualMachineState *other, TR::IlBuilder *b) {
    VirtualMachineState *otherState = (VirtualMachineState *)other;
    _stack->MergeInto(otherState->_stack, b);
    _stackTop->MergeInto(otherState->_stackTop, b);
  }

  OMR::VirtualMachineOperandStack *_stack;
  OMR::VirtualMachineRegister *_stackTop;
};

Compiler::Compiler(VirtualMachine *virtualMachine, const Config &cfg)
    : virtualMachine_(virtualMachine), cfg_(cfg) {
  auto stackElementType = types_.toIlType<StackElement>();

  // Stack
  types_.DefineStruct("executionContextType");
  // types_.DefineField("executionContextType", "stackBase", stackElementPointerType,
  //                   offsetof(Stack, stackBase));
  types_.DefineField("executionContextType", "stackPointer",
                     types_.PointerTo(types_.PointerTo(stackElementType)),
                     offsetof(struct ExecutionContext, stackPointer_));
  types_.CloseStruct("executionContextType");
}

JitFunction Compiler::generateCode(const std::size_t functionIndex) {
  const FunctionSpec *function = virtualMachine_->getFunction(functionIndex);
  MethodBuilder methodBuilder(virtualMachine_, &types_, cfg_, functionIndex);
  if (cfg_.debug)
    std::cout << "MethodBuilder for function: " << function->name
              << " is constructed" << std::endl;
  uint8_t *entry = nullptr;
  int rc = compileMethodBuilder(&methodBuilder, &entry);
  if (rc != 0) {
    std::cout << "Failed to compile function: " << function->name
              << " nargs: " << function->nargs << std::endl;
    throw b9::CompilationException{"IL generation failed"};
  }

  if (cfg_.debug)
    std::cout << "Compilation completed with return code: " << rc
              << ", code address: " << (void *)entry << std::endl;

  return (JitFunction)entry;
}

MethodBuilder::MethodBuilder(VirtualMachine *virtualMachine,
                             TR::TypeDictionary *types, const Config &cfg,
                             const std::size_t functionIndex)
    : TR::MethodBuilder(types),
      virtualMachine_(virtualMachine),
      types_(types),
      cfg_(cfg),
      functionIndex_(functionIndex),
      context_(virtualMachine->executionContext()),
      maxInlineDepth(cfg.maxInlineDepth),
      firstArgumentIndex(0) {
  const FunctionSpec *function = virtualMachine_->getFunction(functionIndex);
  DefineLine(LINETOSTR(__LINE__));
  DefineFile(__FILE__);

  DefineName(function->name.c_str());

  stackElementType = types_->template toIlType<StackElement>();
  stackElementPointerType = types_->PointerTo(stackElementType);

  addressPointerType = types_->PointerTo(Address);
  int64PointerType = types_->PointerTo(Int64);
  int32PointerType = types_->PointerTo(Int32);
  int16PointerType = types_->PointerTo(Int16);

  executionContextType = types_->LookupStruct("executionContextType");
  stackPointerType = types_->PointerTo(executionContextType);

  DefineReturnType(stackElementType);

  defineParameters(function->nargs);

  if (cfg.lazyVmState) {
    if (cfg_.debug) std::cout << "defining localContext\n";
    DefineLocal("localContext", executionContextType);
  }

  defineLocals(function->nargs);

  defineFunctions();

  AllLocalsHaveBeenDefined();
}

static const char *argsAndTempNames[] = {
    "arg00", "arg01", "arg02", "arg03", "arg04", "arg05", "arg06",
    "arg07", "arg08", "arg09", "arg10", "arg11", "arg12", "arg13",
    "arg14", "arg15", "arg16", "arg17", "arg18", "arg19", "arg20",
    "arg21", "arg22", "arg23", "arg24", "arg25", "arg26", "arg27",
    "arg28", "arg29", "arg30", "arg31", "arg32"};
#define MAX_ARGS_TEMPS_AVAIL \
  sizeof(argsAndTempNames) / sizeof(argsAndTempNames[0])

void MethodBuilder::defineParameters(std::size_t argCount) {
  if (cfg_.debug) std::cout << "Defining " << argCount << " parameters\n";
  if (cfg_.passParam) {
    for (int i = 0; i < argCount; i++) {
      DefineParameter(argsAndTempNames[i], stackElementType);
    }
  }
}

void MethodBuilder::defineLocals(std::size_t argCount) {
  if (cfg_.passParam) {
    // for locals we pre-define all the locals we could use, for the toplevel
    // and all the inlined names which are simply referenced via a skew to reach
    // past callers functions args/temps
    const FunctionSpec *function = virtualMachine_->getFunction(functionIndex_);
    std::size_t topLevelLocals = function->nargs + function->nregs;
    if (cfg_.debug) {
      std::cout << "CREATING " << topLevelLocals << " topLevel with "
                << MAX_ARGS_TEMPS_AVAIL - topLevelLocals
                << " slots for inlining\n";
    }

    for (std::size_t i = argCount; i < MAX_ARGS_TEMPS_AVAIL; i++) {
      DefineLocal(argsAndTempNames[i], stackElementType);
    }
  } else {
    DefineLocal("returnSP", Address);
  }
}

void MethodBuilder::defineFunctions() {
  int functionIndex = 0;
  while (functionIndex < virtualMachine_->getFunctionCount()) {
    if (virtualMachine_->getJitAddress(functionIndex) != nullptr) {
      auto function = virtualMachine_->getFunction(functionIndex);
      auto name = function->name.c_str();
      DefineFunction(name, (char *)__FILE__, name,
                     (void*)virtualMachine_->getJitAddress(functionIndex), Int64,
                     function->nargs, stackElementType, stackElementType,
                     stackElementType, stackElementType, stackElementType,
                     stackElementType, stackElementType, stackElementType);
    }
    functionIndex++;
  }

  DefineFunction((char *)"interpret_0", (char *)__FILE__, "interpret_0",
                 (void *)&interpret_0, Int64, 2, addressPointerType,
                 int32PointerType);
  DefineFunction((char *)"interpret_1", (char *)__FILE__, "interpret_1",
                 (void *)&interpret_1, Int64, 3, addressPointerType,
                 int32PointerType, stackElementType);
  DefineFunction((char *)"interpret_2", (char *)__FILE__, "interpret_2",
                 (void *)&interpret_2, Int64, 4, addressPointerType,
                 int32PointerType, stackElementType, stackElementType);
  DefineFunction((char *)"interpret_3", (char *)__FILE__, "interpret_3",
                 (void *)&interpret_3, Int64, 5, addressPointerType,
                 int32PointerType, stackElementType, stackElementType,
                 stackElementType);
  DefineFunction((char *)"primitive_call", (char *)__FILE__, "primitive_call",
                 (void *)&primitive_call, NoType, 2, addressPointerType, Int32);
  DefineFunction((char*)"b9PrintStack", (char*)__FILE__, "b9PrintStack", 
                 (void*)&b9PrintStack, NoType, 4, addressPointerType, Int64, Int64, Int64);
}

#define QSTACK(b) (((VirtualMachineState *)(b)->vmState())->_stack)
#define QCOMMIT(b) \
  if (cfg_.lazyVmState) ((b)->vmState()->Commit(b));
#define QRELOAD(b) \
  if (cfg_.lazyVmState) ((b)->vmState()->Reload(b));
#define QRELOAD_DROP(b, toDrop) \
  if (cfg_.lazyVmState) QSTACK(b)->Drop(b, toDrop);

long computeNumberOfBytecodes(const Instruction *program) {
  long result = 0;
  while (*program != END_SECTION) {
    program++;
    result++;
  }
  return result;
}

bool MethodBuilder::inlineProgramIntoBuilder(
    const std::size_t functionIndex,
    TR::BytecodeBuilder *currentBuilder,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {

  bool success = true;
  maxInlineDepth--;
  const FunctionSpec *function = virtualMachine_->getFunction(functionIndex);
  const Instruction *program = function->address;

  // Create a BytecodeBuilder for each Bytecode
  auto numberOfBytecodes = computeNumberOfBytecodes(program);
  if (cfg_.debug)
    std::cout << "Creating " << numberOfBytecodes << " bytecode builders\n";
  std::vector<TR::BytecodeBuilder *> builderTable;
  for (int i = 0; i < numberOfBytecodes; i++) {
    builderTable.push_back(OrphanBytecodeBuilder(i));
  }

  // Get the first Builder
  TR::BytecodeBuilder *builder = builderTable[0];
  currentBuilder->AddFallThroughBuilder(builder);

  // Create a BytecodeBuilder for each Bytecode
  for (int i = 0; i < numberOfBytecodes; i++) {
    ByteCode bc = program[i].byteCode();
    if (!generateILForBytecode(functionIndex, builderTable, program, bc, i,
                               jumpToBuilderForInlinedReturn)) {
      success = false;
      break;
    }
  }

  maxInlineDepth++;
  return success;
}

bool MethodBuilder::buildIL() {

  // Create the lazy virtual machine state if enabled
  if (cfg_.lazyVmState) {
    if (cfg_.debug) std::cout << "setting vmstate in buildIL\n";
    this->Store("localContext", this->ConstAddress(context_));
    OMR::VirtualMachineRegisterInStruct *stackTop =
        new OMR::VirtualMachineRegisterInStruct(this, "executionContextType", "localContext",
                                                "stackPointer", "SP");
    OMR::VirtualMachineOperandStack *stack =
        new OMR::VirtualMachineOperandStack(this, 32, stackElementPointerType,
                                            stackTop, true, 0);
    VirtualMachineState *vms = new VirtualMachineState(stack, stackTop);
    setVMState(vms);
  } else {
    setVMState(new OMR::VirtualMachineState());
  }

  const FunctionSpec *function = virtualMachine_->getFunction(functionIndex_);
  auto builder = OrphanBytecodeBuilder();
  AppendBuilder(builder);

  // Initialize parameters and locals on the stack.
  if (cfg_.passParam) {
    // Arguments and locals are not stored on the stack. Arguments are passed
    // on the native stack, and locals are stored as JitBuilder locals
    int argsCount = function->nargs;
    int regsCount = function->nregs;
    for (int i = argsCount; i < argsCount + regsCount; i++) {
      storeVarIndex(builder, i,
          builder->ConstInt64(0));  // init all temps to zero
    }
  } else {
    // Arguments and locals are passed on the stack.
    // arguments are &sp[-number_of_args]
    // temps are pushes onto the stack to &sp[number_of_temps]
    QCOMMIT(builder);
    TR::IlValue *sp = builder->LoadIndirect("executionContextType", "stackPointer",
        builder->ConstAddress(context_));

    TR::IlValue *args =
      builder->IndexAt(stackElementPointerType, sp,
          builder->ConstInt32(0 - function->nargs));

    builder->Store("returnSP", args);

    TR::IlValue *newSP = builder->IndexAt(
        stackElementPointerType, sp, builder->ConstInt32(function->nregs));

    builder->StoreIndirect("executionContextType", "stackPointer",
        builder->ConstAddress(context_), newSP);
    QRELOAD(builder);
    QCOMMIT(builder);
    if (cfg_.debug) builder->Call("b9PrintStack", 4, builder->ConstAddress(virtualMachine_->executionContext()),
                                        builder->ConstInt64(-1),
                                        builder->ConstInt64(-1),
                                        builder->ConstInt64(0));
  }

  auto returnBuilder = OrphanBytecodeBuilder();
  auto success = inlineProgramIntoBuilder(functionIndex_, builder, returnBuilder);

  if (success) {
    // Create a builder to return from the current function.  This is the last
    // builder which will be run in the compiled function.
    QCOMMIT(returnBuilder);
    auto result = pop(returnBuilder);
    if (!cfg_.passParam) {
      returnBuilder->StoreIndirect("executionContextType", "stackPointer",
          returnBuilder->ConstAddress(context_),
          returnBuilder->Load("returnSP"));
    }
    returnBuilder->Return(result);
  }

  return success;
}

TR::IlValue *MethodBuilder::loadVarIndex(TR::BytecodeBuilder *builder,
                                         int varindex) {
  if (firstArgumentIndex > 0) {
    // if (context->debug >= 2) {
    //   printf("loadVarIndex varindex adjusted = %d %d\n", varindex,
    //          firstArgumentIndex);
    // }

    varindex += firstArgumentIndex;
  }

  if (cfg_.passParam) {
    return builder->Load(argsAndTempNames[varindex]);
  } else {
    TR::IlValue *args = builder->Load("returnSP");
    TR::IlValue *address = builder->IndexAt(stackElementPointerType, args,
                                            builder->ConstInt32(varindex));
    TR::IlValue *result = builder->LoadAt(stackElementPointerType, address);
    result = builder->ConvertTo(stackElementType, result);
    return result;
  }
}

void MethodBuilder::storeVarIndex(TR::BytecodeBuilder *builder, int varindex,
                                  TR::IlValue *value) {
  if (firstArgumentIndex > 0) {
    // if (context->debug >= 2) {
    //   printf("storeVarIndex varindex adjusted = %d %d\n", varindex,
    //          firstArgumentIndex);
    // }
    varindex += firstArgumentIndex;
  }
  if (cfg_.passParam) {
    builder->Store(argsAndTempNames[varindex], value);
    return;
  } else {
    TR::IlValue *args = builder->Load("returnSP");
    TR::IlValue *address = builder->IndexAt(stackElementPointerType, args,
                                            builder->ConstInt32(varindex));
    builder->StoreAt(address, value);
  }
}

bool MethodBuilder::generateILForBytecode(
    const std::size_t functionIndex,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, ByteCode bytecode, long bytecodeIndex,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {

  TR::BytecodeBuilder *builder = bytecodeBuilderTable[bytecodeIndex];
  Instruction instruction = program[bytecodeIndex];
  assert(bytecode == instruction.byteCode());

  if (NULL == builder) {
    if (cfg_.debug) std::cout << "unexpected NULL BytecodeBuilder!\n";
    return false;
  }

  auto numberOfBytecodes = computeNumberOfBytecodes(program);
  TR::BytecodeBuilder *nextBytecodeBuilder = nullptr;
  int32_t nextBytecodeIndex = bytecodeIndex + 1;
  if (nextBytecodeIndex < numberOfBytecodes) {
    nextBytecodeBuilder = bytecodeBuilderTable[nextBytecodeIndex];
  }

  bool handled = true;
  const FunctionSpec *function = virtualMachine_->getFunction(functionIndex);

  if (cfg_.debug) {
    if (jumpToBuilderForInlinedReturn) {
      std::cout << "INLINED METHOD: skew " << firstArgumentIndex
                << " return bc will jump to " << jumpToBuilderForInlinedReturn
                << ": ";
    }
    if (cfg_.debug) std::cout << "generating index=" << bytecodeIndex << " bc=" << instruction
              << std::endl;
    QCOMMIT(builder);
    if (cfg_.debug) builder->Call("b9PrintStack", 4, builder->ConstAddress(virtualMachine_->executionContext()),
					builder->ConstInt64(bytecodeIndex),
					builder->ConstInt64((int) bytecode),
					builder->ConstInt64(instruction.parameter()));
  }

  switch (bytecode) {
    case ByteCode::PUSH_FROM_VAR:
      push(builder, loadVarIndex(builder, instruction.parameter()));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case ByteCode::POP_INTO_VAR:
      storeVarIndex(builder, instruction.parameter(), pop(builder));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case ByteCode::FUNCTION_RETURN:
      builder->Goto(jumpToBuilderForInlinedReturn);
      break;
    case ByteCode::DROP:
      drop(builder);
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case ByteCode::JMP:
      handle_bc_jmp(builder, bytecodeBuilderTable, program, bytecodeIndex);
      break;
    case ByteCode::INT_JMP_EQ:
      handle_bc_jmp_eq(builder, bytecodeBuilderTable, program, bytecodeIndex,
                       nextBytecodeBuilder);
      break;
    case ByteCode::INT_JMP_NEQ:
      handle_bc_jmp_neq(builder, bytecodeBuilderTable, program, bytecodeIndex,
                        nextBytecodeBuilder);
      break;
    case ByteCode::INT_JMP_LT:
      handle_bc_jmp_lt(builder, bytecodeBuilderTable, program, bytecodeIndex,
                       nextBytecodeBuilder);
      break;
    case ByteCode::INT_JMP_LE:
      handle_bc_jmp_le(builder, bytecodeBuilderTable, program, bytecodeIndex,
                       nextBytecodeBuilder);
      break;
    case ByteCode::INT_JMP_GT:
      handle_bc_jmp_gt(builder, bytecodeBuilderTable, program, bytecodeIndex,
                       nextBytecodeBuilder);
      break;
    case ByteCode::INT_JMP_GE:
      handle_bc_jmp_ge(builder, bytecodeBuilderTable, program, bytecodeIndex,
                       nextBytecodeBuilder);
      break;
    case ByteCode::INT_SUB:
      handle_bc_sub(builder, nextBytecodeBuilder);
      break;
    case ByteCode::INT_ADD:
      handle_bc_add(builder, nextBytecodeBuilder);
      break;
    case ByteCode::INT_PUSH_CONSTANT: {
      int constvalue = instruction.parameter();
      push(builder, builder->ConstInt64(constvalue));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case ByteCode::STR_PUSH_CONSTANT: {
      int index = instruction.parameter();
      push(builder, builder->ConstInt64(
                        (int64_t)(char *)virtualMachine_->getString(index)));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case ByteCode::PRIMITIVE_CALL: {
      if (cfg_.debug) std::cout << "qcommitting\n";
      QCOMMIT(builder);
      TR::IlValue *result = builder->Call(
          "primitive_call", 2,
          builder->ConstAddress(virtualMachine_->executionContext()),
          builder->ConstInt32(instruction.parameter()));
      if (cfg_.debug) std::cout << "qdropping 1 slot\n";
      //QRELOAD_DROP(builder, 1);
      QRELOAD(builder);
      //push(builder, result);
      //push(builder, builder->ConstInt32(0));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case ByteCode::FUNCTION_CALL: {
      const std::size_t callindex = instruction.parameter();
      const FunctionSpec *callee = virtualMachine_->getFunction(callindex);
      const Instruction *tocall = callee->address;
      const std::uint32_t argsCount = callee->nargs;
      const std::uint32_t regsCount = callee->nregs;

      if (cfg_.directCall) {
        if (cfg_.debug)
          std::cout << "Handling direct calls to " << callee->name
                    << std::endl;
        const char *interpretName[] = {"interpret_0", "interpret_1",
                                       "interpret_2", "interpret_3"};
        const char *nameToCall = interpretName[argsCount];
        bool interp = true;
        if (tocall == program ||
            virtualMachine_->getJitAddress(callindex) != 0) {
          nameToCall = callee->name.c_str();
          interp = false;
        }

        if (cfg_.passParam) {
          if (cfg_.debug) {
            std::cout << "Parameters are passed to the function call\n";
          }
          if (maxInlineDepth >= 0 && !interp) {
            int32_t save = firstArgumentIndex;
            int32_t skipLocals = function->nargs + function->nregs;
            int32_t spaceNeeded = argsCount + regsCount;
            firstArgumentIndex += skipLocals;
            // no need to define locals here the outer program registered all
            // locals
            // it means some locals will be reused which will affect liveness of
            // a
            // variable
            if ((firstArgumentIndex + spaceNeeded) < MAX_ARGS_TEMPS_AVAIL) {
              // printf("INLINING RECURSION ONLY  old skew %d, new skew = %d\n",
              // save, firstArgumentIndex);
              int storeInto = argsCount;
              while (storeInto-- > 0) {
                // printf("Storing temp %d into dest variable \n", storeInto);
                storeVarIndex(builder, storeInto,
                              pop(builder));  // firstArgumentIndex is added in
                                              // storeVarIndex
              }
              bool result = inlineProgramIntoBuilder(callindex, builder,
                                                     nextBytecodeBuilder);
              if (!result) {
                std::cerr << "Failed inlineProgramIntoBuilder" << std::endl;
                return result;
              } else {
                if (cfg_.debug)
                  std::cout << "Successfully inlined: " << callee->name << std::endl;
              }
              // printf("SETTING SKEW BACK from %d to %d\n", firstArgumentIndex,
              // save);
              firstArgumentIndex = save;
              break;
            } else {
              std::cerr << "SKIP INLINE DUE TO EXCESSIVE TEMPS NEEDED" << std::endl;
            }
          }
          if (argsCount > 8) {
            throw std::runtime_error{"Need to add handlers for more parameters"};
            break;
          }
          TR::IlValue *p[8];
          memset(p, 0, sizeof(p));
          int popInto = argsCount;
          while (popInto--) {
            p[popInto] = pop(builder);
          }
          if (interp) {
            TR::IlValue *result = builder->Call(
                nameToCall, 2 + argsCount,
                builder->ConstAddress(virtualMachine_->executionContext()),
                builder->ConstInt32(callindex), p[0], p[1], p[2], p[3], p[4],
                p[5], p[6], p[7]);
            push(builder, result);
          } else {
            TR::IlValue *result =
                builder->Call(nameToCall, argsCount, p[0], p[1], p[2], p[3],
                              p[4], p[5], p[6], p[7]);
            push(builder, result);
          }
        } else {
          if (cfg_.debug) {
            std::cout << "Parameters are on stack to the function call\n";
          }
          TR::IlValue *result;
          if (cfg_.debug) std::cout << "qcommitting\n";
          QCOMMIT(builder);
          if (interp) {
            if (cfg_.debug)
              std::cout << "calling interpreter " << nameToCall << std::endl;
            result = builder->Call(
                nameToCall, 2,
                builder->ConstAddress(virtualMachine_->executionContext()),
                builder->ConstInt32(callindex));
          } else {
            if (cfg_.debug)
              std::cout << "calling " << nameToCall << " directly\n";
            result = builder->Call(nameToCall, 0);
          }
          if (cfg_.debug) std::cout << "qdropping " << argsCount << " slots\n";
          QRELOAD_DROP(builder, argsCount);
          //QRELOAD(builder);
          push(builder, result);
        }
      } else {
        // only use interpreter to dispatch the calls
        if (cfg_.debug)
          std::cout << "Calling interpret_0 to dispatch call for "
                    << callee->name << " with " << argsCount << " args\n";
        if (cfg_.debug) std::cout << "qcommitting\n";
        QCOMMIT(builder);
        TR::IlValue *result = builder->Call(
            "interpret_0", 2,
            builder->ConstAddress(virtualMachine_->executionContext()),
            builder->ConstInt32(callindex));
        if (cfg_.debug) std::cout << "qdropping " << argsCount << " slots\n";
        QRELOAD_DROP(builder, argsCount);
        //QRELOAD(builder);
        push(builder, result);
      }

      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    default:
      if (cfg_.debug) {
        std::cout << "Cannot handle unknown bytecode: returning\n";
      }
      handled = false;
      break;
  }

  return handled;
}

/*************************************************
 * GENERATE CODE FOR BYTECODES
 *************************************************/

void MethodBuilder::handle_bc_jmp(
    TR::BytecodeBuilder *builder,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, long bytecodeIndex) {
  Instruction instruction = program[bytecodeIndex];
  StackElement delta = instruction.parameter() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *destBuilder = bytecodeBuilderTable[next_bc_index];
  builder->Goto(destBuilder);
}

void MethodBuilder::handle_bc_jmp_eq(
    TR::BytecodeBuilder *builder,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  StackElement delta = instruction.parameter() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  builder->IfCmpEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_neq(
    TR::BytecodeBuilder *builder,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  StackElement delta = instruction.parameter() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  builder->IfCmpNotEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_lt(
    TR::BytecodeBuilder *builder,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  StackElement delta = instruction.parameter() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  builder->IfCmpLessThan(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_le(
    TR::BytecodeBuilder *builder,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  StackElement delta = instruction.parameter() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  builder->IfCmpLessOrEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_gt(
    TR::BytecodeBuilder *builder,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  StackElement delta = instruction.parameter() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  builder->IfCmpGreaterThan(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_ge(
    TR::BytecodeBuilder *builder,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  StackElement delta = instruction.parameter() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  builder->IfCmpGreaterOrEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_sub(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  push(builder, builder->Sub(left, right));
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_add(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  TR::IlValue *right = pop(builder);
  TR::IlValue *left = pop(builder);

  push(builder, builder->Add(left, right));
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::drop(TR::BytecodeBuilder *builder) { pop(builder); }

TR::IlValue *MethodBuilder::pop(TR::BytecodeBuilder *builder) {
  if (cfg_.lazyVmState) {
    if (cfg_.debug) std::cout << "-1: popping stack\n";
    return QSTACK(builder)->Pop(builder);
  } else {
    TR::IlValue *sp = builder->LoadIndirect("executionContextType", "stackPointer",
                                            builder->ConstAddress(context_));
    TR::IlValue *newSP =
        builder->IndexAt(stackElementPointerType, sp, builder->ConstInt32(-1));
    builder->StoreIndirect("executionContextType", "stackPointer",
                           builder->ConstAddress(context_), newSP);
    return builder->LoadAt(stackElementPointerType, newSP);
  }
}

void MethodBuilder::push(TR::BytecodeBuilder *builder, TR::IlValue *value) {
  if (cfg_.lazyVmState) {
    if (cfg_.debug) std::cout << "+1: pushing stack\n";
    QSTACK(builder)->Push(builder, value);
  } else {
    TR::IlValue *sp = builder->LoadIndirect("executionContextType", "stackPointer",
                                            builder->ConstAddress(context_));
    builder->StoreAt(builder->ConvertTo(stackElementPointerType, sp),
                     builder->ConvertTo(stackElementType, value));
    TR::IlValue *newSP =
        builder->IndexAt(stackElementPointerType, sp, builder->ConstInt32(1));
    builder->StoreIndirect("executionContextType", "stackPointer",
                           builder->ConstAddress(context_), newSP);
  }
}

}  // namespace b9
