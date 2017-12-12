#include "b9/jit.hpp"
#include "b9/instructions.hpp"
#include "b9/interpreter.hpp"

#include "Jit.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include <dlfcn.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace b9 {

extern "C" void b9PrintStack(ExecutionContext *context, int64_t pc,
                             int64_t bytecode, int64_t param) {
  StackElement *base = context->stackBase_;
  std::cout << "Executing at pc: " << pc
            << " bc: " << toString((ByteCode)bytecode) << ", " << param
            << std::endl;
  std::cout << "vvvvvvvvvvvvvvvvv" << std::endl;
  while (base < context->stackPointer_) {
    std::cout << base << ": Stack[" << base - context->stackBase_
              << "] = " << *base << std::endl;
    base++;
  }
  std::cout << "^^^^^^^^^^^^^^^^^" << std::endl;
}

// Simulates all state of the virtual machine state while compiled code is
// running. It simulates the stack and the pointer to the top of the stack.
class VirtualMachineState : public OMR::VirtualMachineState {
 public:
  VirtualMachineState() = default;

  VirtualMachineState(OMR::VirtualMachineOperandStack *stack,
                      OMR::VirtualMachineRegister *stackTop)
      : _stack(stack), _stackTop(stackTop) {}

  void Commit(TR::IlBuilder *b) override {
    _stack->Commit(b);
    _stackTop->Commit(b);
  }

  void Reload(TR::IlBuilder *b) override {
    _stackTop->Reload(b);
    _stack->Reload(b);
  }

  VirtualMachineState *MakeCopy() override {
    auto newState = new VirtualMachineState();
    newState->_stack =
        dynamic_cast<OMR::VirtualMachineOperandStack *>(_stack->MakeCopy());
    newState->_stackTop =
        dynamic_cast<OMR::VirtualMachineRegister *>(_stackTop->MakeCopy());
    return newState;
  }

  void MergeInto(OMR::VirtualMachineState *other, TR::IlBuilder *b) override {
    auto otherState = dynamic_cast<VirtualMachineState *>(other);
    _stack->MergeInto(otherState->_stack, b);
    _stackTop->MergeInto(otherState->_stackTop, b);
  }

  OMR::VirtualMachineOperandStack *_stack = nullptr;
  OMR::VirtualMachineRegister *_stackTop = nullptr;
};

Compiler::Compiler(VirtualMachine *virtualMachine, const Config &cfg)
    : virtualMachine_(virtualMachine), cfg_(cfg) {
  auto stackElementType = types_.toIlType<StackElement>();

  // Stack
  types_.DefineStruct("executionContextType");
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
  auto rc = compileMethodBuilder(&methodBuilder, &entry);
  if (rc != 0) {
    std::cout << "Failed to compile function: " << function->name
              << " nargs: " << function->nargs << std::endl;
    throw b9::CompilationException{"IL generation failed"};
  }

  if (cfg_.debug)
    std::cout << "Compilation completed with return code: " << rc
              << ", code address: " << entry << std::endl;

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
  (sizeof(argsAndTempNames) / sizeof(argsAndTempNames[0]))

void MethodBuilder::defineParameters(std::size_t argCount) {
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
                << " slots for inlining" << std::endl;
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
                     (void *)virtualMachine_->getJitAddress(functionIndex),
                     Int64, function->nargs, stackElementType, stackElementType,
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
  DefineFunction((char *)"b9PrintStack", (char *)__FILE__, "b9PrintStack",
                 (void *)&b9PrintStack, NoType, 4, addressPointerType, Int64,
                 Int64, Int64);
}

#define QSTACK(b) (((VirtualMachineState *)(b)->vmState())->_stack)
#define QRELOAD(b) \
  if (cfg_.lazyVmState) ((b)->vmState()->Reload(b));
#define QRELOAD_DROP(b, toDrop) \
  if (cfg_.lazyVmState) QSTACK(b)->Drop(b, toDrop);

uint64_t computeNumberOfBytecodes(const Instruction *program) {
  uint64_t result = 0;
  while (*program != END_SECTION) {
    program++;
    result++;
  }
  return result;
}

bool MethodBuilder::inlineProgramIntoBuilder(
    const std::size_t functionIndex, bool isTopLevel,
    TR::BytecodeBuilder *currentBuilder,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {
  bool success = true;
  maxInlineDepth--;
  const FunctionSpec *function = virtualMachine_->getFunction(functionIndex);
  const Instruction *program = function->instructions.data();

  // Create a BytecodeBuilder for each Bytecode
  auto numberOfBytecodes = computeNumberOfBytecodes(program);
  if (numberOfBytecodes == 0) {
    if (cfg_.debug) {
      std::cout << "unexpected EMPTY function body for " << function->name
                << std::endl;
    }
    return false;
  }

  if (cfg_.debug)
    std::cout << "Creating " << numberOfBytecodes << " bytecode builders" << std::endl;
  std::vector<TR::BytecodeBuilder *> builderTable;
  builderTable.reserve(numberOfBytecodes);
  for (int i = 0; i < numberOfBytecodes; i++) {
    builderTable.push_back(OrphanBytecodeBuilder(i));
  }

  // Get the first Builder
  TR::BytecodeBuilder *builder = builderTable[0];

  if (isTopLevel) {
    AppendBuilder(builder);
  } else {
    currentBuilder->AddFallThroughBuilder(builder);
  }

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
  // Set up arguments and locals, and adjust stack pointer if needed
  const FunctionSpec *function = virtualMachine_->getFunction(functionIndex_);
  if (cfg_.passParam) {
    int argsCount = function->nargs;
    int regsCount = function->nregs;
    for (int i = argsCount; i < argsCount + regsCount; i++) {
      storeVarIndex(this, i, this->ConstInt64(0));  // init all temps to zero
    }
  } else {
    // arguments are &sp[-number_of_args]
    // temps are pushes onto the stack to &sp[number_of_temps]
    TR::IlValue *sp = this->LoadIndirect("executionContextType", "stackPointer",
                                         this->ConstAddress(context_));
    TR::IlValue *args = this->IndexAt(stackElementPointerType, sp,
                                      this->ConstInt32(0 - function->nargs));
    this->Store("returnSP", args);
    TR::IlValue *newSP = this->IndexAt(stackElementPointerType, sp,
                                       this->ConstInt32(function->nregs));
    this->StoreIndirect("executionContextType", "stackPointer",
                        this->ConstAddress(context_), newSP);
  }

  if (cfg_.lazyVmState) {
    this->Store("localContext", this->ConstAddress(context_));
    auto stackTop = new OMR::VirtualMachineRegisterInStruct(
        this, "executionContextType", "localContext", "stackPointer", "SP");
    auto stack = new OMR::VirtualMachineOperandStack(
        this, 32, stackElementPointerType, stackTop, true, 0);
    auto vms = new VirtualMachineState(stack, stackTop);
    setVMState(vms);
  } else {
    setVMState(new OMR::VirtualMachineState());
  }

  return inlineProgramIntoBuilder(functionIndex_, true);
}

TR::IlValue *MethodBuilder::loadVarIndex(TR::IlBuilder *builder, int varindex) {
  if (firstArgumentIndex > 0) {
    varindex += firstArgumentIndex;
  }

  TR::IlValue *result = nullptr;

  if (cfg_.passParam) {
    result = builder->Load(argsAndTempNames[varindex]);
  } else {
    TR::IlValue *args = builder->Load("returnSP");
    TR::IlValue *address = builder->IndexAt(stackElementPointerType, args,
                                            builder->ConstInt32(varindex));
    result = builder->LoadAt(stackElementPointerType, address);
    result = builder->ConvertTo(stackElementType, result);
  }

  return result;
}

void MethodBuilder::storeVarIndex(TR::IlBuilder *builder, int varindex,
                                  TR::IlValue *value) {
  if (firstArgumentIndex > 0) {
    varindex += firstArgumentIndex;
  }

  if (cfg_.passParam) {
    builder->Store(argsAndTempNames[varindex], value);
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

  if (nullptr == builder) {
    if (cfg_.debug) std::cout << "unexpected NULL BytecodeBuilder!" << std::endl;
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
    if (jumpToBuilderForInlinedReturn != nullptr) {
      std::cout << "INLINED METHOD: skew " << firstArgumentIndex
                << " return bc will jump to " << jumpToBuilderForInlinedReturn
                << ": ";
    }
    std::cout << "generating index=" << bytecodeIndex << " bc=" << instruction
              << std::endl;
    builder->vmState()->Commit(builder);
    builder->Call("b9PrintStack", 4,
                  builder->ConstAddress(virtualMachine_->executionContext()),
                  builder->ConstInt64(bytecodeIndex),
                  builder->ConstInt64((int)bytecode),
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
    case ByteCode::FUNCTION_RETURN: {
      if (jumpToBuilderForInlinedReturn) {
        builder->Goto(jumpToBuilderForInlinedReturn);
      } else {
        builder->vmState()->Commit(builder);
        auto result = pop(builder);
        if (!cfg_.passParam) {
          builder->StoreIndirect("executionContextType", "stackPointer",
                                 builder->ConstAddress(context_),
                                 builder->Load("returnSP"));
        }
        builder->Return(result);
      }
    } break;
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
    // CASCON2017 - add INT_MUL and INT_DIV here
    case ByteCode::INT_NOT:
      handle_bc_not(builder, nextBytecodeBuilder);
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
      builder->vmState()->Commit(builder);
      TR::IlValue *result = builder->Call(
          "primitive_call", 2,
          builder->ConstAddress(virtualMachine_->executionContext()),
          builder->ConstInt32(instruction.parameter()));
      QRELOAD(builder);
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case ByteCode::FUNCTION_CALL: {
      const std::size_t callindex = instruction.parameter();
      const FunctionSpec *callee = virtualMachine_->getFunction(callindex);
      const Instruction *tocall = callee->instructions.data();
      const std::uint32_t argsCount = callee->nargs;
      const std::uint32_t regsCount = callee->nregs;

      if (cfg_.directCall) {
        if (cfg_.debug)
          std::cout << "Handling direct calls to " << callee->name << std::endl;
        const char *interpretName[] = {"interpret_0", "interpret_1",
                                       "interpret_2", "interpret_3"};
        const char *nameToCall = interpretName[argsCount];
        bool interp = true;
        if (tocall == program ||
            virtualMachine_->getJitAddress(callindex) != nullptr) {
          nameToCall = callee->name.c_str();
          interp = false;
        }

        if (cfg_.passParam) {
          if (cfg_.debug) {
            std::cout << "Parameters are passed to the function call" << std::endl;
          }

          // Attempt to inline the function we're calling
          if (maxInlineDepth >= 0 && !interp) {
            int32_t save = firstArgumentIndex;
            int32_t skipLocals = function->nargs + function->nregs;
            int32_t spaceNeeded = argsCount + regsCount;
            firstArgumentIndex += skipLocals;
            // no need to define locals here, the outer program registered all
            // locals. it means some locals will be reused which will affect
            // liveness of a variable
            if ((firstArgumentIndex + spaceNeeded) < MAX_ARGS_TEMPS_AVAIL) {
              int storeInto = argsCount;
              while (storeInto-- > 0) {
                // firstArgumentIndex is added in storeVarIndex
                storeVarIndex(builder, storeInto, pop(builder));
              }

              bool result = inlineProgramIntoBuilder(callindex, false, builder,
                                                     nextBytecodeBuilder);
              if (!result) {
                std::cerr << "Failed inlineProgramIntoBuilder" << std::endl;
                return result;
              }

              if (cfg_.debug)
                std::cout << "Successfully inlined: " << callee->name
                          << std::endl;
              firstArgumentIndex = save;
              break;
            }
            std::cerr << "SKIP INLINE DUE TO EXCESSIVE TEMPS NEEDED"
                      << std::endl;
          }

          if (argsCount > 8) {
            throw std::runtime_error{
                "Need to add handlers for more parameters"};
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
            std::cout << "Parameters are on stack to the function call" << std::endl;
          }
          TR::IlValue *result;
          builder->vmState()->Commit(builder);
          if (interp) {
            if (cfg_.debug)
              std::cout << "calling interpreter " << nameToCall << std::endl;
            result = builder->Call(
                nameToCall, 2,
                builder->ConstAddress(virtualMachine_->executionContext()),
                builder->ConstInt32(callindex));
          } else {
            if (cfg_.debug)
              std::cout << "calling " << nameToCall << " directly" << std::endl;
            result = builder->Call(nameToCall, 0);
          }
          QRELOAD_DROP(builder, argsCount);
          push(builder, result);
        }
      } else {
        // only use interpreter to dispatch the calls
        if (cfg_.debug)
          std::cout << "Calling interpret_0 to dispatch call for "
                    << callee->name << " with " << argsCount << " args" << std::endl;
        builder->vmState()->Commit(builder);
        TR::IlValue *result = builder->Call(
            "interpret_0", 2,
            builder->ConstAddress(virtualMachine_->executionContext()),
            builder->ConstInt32(callindex));
        QRELOAD_DROP(builder, argsCount);
        push(builder, result);
      }

      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    default:
      if (cfg_.debug) {
        std::cout << "Cannot handle unknown bytecode: returning" << std::endl;
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

// CASCON2017 - add handle_bc_mul and handle_bc_div here

void MethodBuilder::handle_bc_not(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  auto zero = builder->ConstInteger(stackElementType, 0);
  auto value = pop(builder);
  auto result =
      builder->ConvertTo(stackElementType, builder->EqualTo(value, zero));
  push(builder, result);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::drop(TR::BytecodeBuilder *builder) { pop(builder); }

TR::IlValue *MethodBuilder::pop(TR::BytecodeBuilder *builder) {
  if (cfg_.lazyVmState) {
    return dynamic_cast<VirtualMachineState *>(builder->vmState())->_stack->Pop(builder);
  }
  TR::IlValue *sp = builder->LoadIndirect(
      "executionContextType", "stackPointer", builder->ConstAddress(context_));
  TR::IlValue *newSP =
      builder->IndexAt(stackElementPointerType, sp, builder->ConstInt32(-1));
  builder->StoreIndirect("executionContextType", "stackPointer",
                         builder->ConstAddress(context_), newSP);
  return builder->LoadAt(stackElementPointerType, newSP);
}

void MethodBuilder::push(TR::BytecodeBuilder *builder, TR::IlValue *value) {
  if (cfg_.lazyVmState) {
    dynamic_cast<VirtualMachineState *>(builder->vmState())->_stack->Push(builder, value);
  } else {
    TR::IlValue *sp =
        builder->LoadIndirect("executionContextType", "stackPointer",
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
