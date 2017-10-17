#include <b9.hpp>

#include <b9.hpp>
#include <b9/bytecodes.hpp>
#include <b9/core.hpp>
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
#include <vector>

namespace b9 {

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

Compiler::Compiler(VirtualMachine *virtualMachine, const JitConfig &jitConfig)
    : virtualMachine_(virtualMachine), jitConfig_(jitConfig) {
  auto stackElementType = types_.toIlType<StackElement>();
  auto stackElementPointerType = types_.PointerTo(stackElementType);

  // Stack
  types_.DefineStruct("Stack");
  types_.DefineField("Stack", "stackBase", stackElementPointerType,
                     offsetof(Stack, stackBase));
  types_.DefineField("Stack", "stackPointer", stackElementPointerType,
                     offsetof(Stack, stackPointer));
  types_.CloseStruct("Stack");
}

uint8_t *Compiler::generateCode(const FunctionSpec &functionSpec) {
  // TODO get the stack
  Stack *stack = nullptr;
  MethodBuilder methodBuilder(virtualMachine_, &types_, jitConfig_,
                              functionSpec, stack);
  uint8_t *entry = nullptr;
  int rc = compileMethodBuilder(&methodBuilder, &entry);
  if (rc != 0) {
    // TODO print an error
    // printf("Failed to compile method \"%d\"\n", functionIndex);
  }

  return entry;
}

MethodBuilder::MethodBuilder(VirtualMachine *virtualMachine,
                             TR::TypeDictionary *types, const JitConfig &config,
                             const FunctionSpec &functionSpec, Stack *stack)
    : TR::MethodBuilder(types),
      virtualMachine_(virtualMachine),
      types_(types),
      config_(config),
      functionSpec_(functionSpec),
      stack_(stack),
      maxInlineDepth(config.maxInlineDepth),
      firstArgumentIndex(0) {
  DefineLine(LINETOSTR(__LINE__));
  DefineFile(__FILE__);

  // TODO
  // DefineName(context->functions[programIndex].name);
  DefineName("FIXME");

  stackElementType = types->template toIlType<StackElement>();
  stackElementPointerType = types->PointerTo(stackElementType);
  int64PointerType = types->PointerTo(Int64);
  int32PointerType = types->PointerTo(Int32);
  int16PointerType = types->PointerTo(Int16);
  stackType = types->LookupStruct("Stack");
  stackPointerType = types->PointerTo(stackType);

  DefineReturnType(stackElementType);

  defineParameters(functionSpec.nargs);

  if (config.operandStack) {
    // hack for topLevel
    DefineLocal("stack", stackType);
  }

  defineLocals(functionSpec.nregs);

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
  if (config_.callStyle == CallStyle::passParameter) {
    for (int i = 0; i < argCount; i++) {
      DefineParameter(argsAndTempNames[i], stackElementType);
    }
  }
}

void MethodBuilder::defineLocals(std::size_t argCount) {
  if (config_.callStyle == CallStyle::passParameter) {
    // for locals we pre-define all the locals we could use, for the toplevel
    // and all the inlined names which are simply referenced via a skew to reach
    // past callers functions args/temps
    std::size_t totalArgCount = functionSpec_.nargs + functionSpec_.nregs;
    for (std::size_t i = totalArgCount; i < MAX_ARGS_TEMPS_AVAIL; i++) {
      DefineLocal(argsAndTempNames[i], stackElementType);
    }
  } else {
    DefineLocal("returnSP", Int64);
  }
}

void MethodBuilder::defineFunctions() {
  // DefineFunction((char *)"printVMState", (char *)__FILE__, "printVMState",
  //                (void *)&printVMState, NoType, 4, Int64, Int64, Int64,
  //                Int64);
  // DefineFunction((char *)"printStack", (char *)__FILE__, "printStack",
  //                (void *)&b9PrintStack, NoType, 1, Int64);
  // DefineFunction((char *)"interpret", (char *)__FILE__, "interpret",
  //                (void *)&interpret, Int64, 2, executionContextType,
  //                int32PointerType);
  // int functionIndex = 0;
  // while (context->functions[functionIndex].name != NO_MORE_FUNCTIONS) {
  //   if (context->functions[functionIndex].jitAddress) {
  //     DefineFunction(context->functions[functionIndex].name, (char
  //     *)__FILE__,
  //                    context->functions[functionIndex].name,
  //                    (void *)context->functions[functionIndex].jitAddress,
  //                    Int64,
  //                    progArgCount(*context->functions[functionIndex].program),
  //                    stackElementType, stackElementType, stackElementType,
  //                    stackElementType, stackElementType, stackElementType,
  //                    stackElementType, stackElementType);
  //   }
  //   functionIndex++;
  // }
  // DefineFunction((char *)"interpret_0", (char *)__FILE__, "interpret_0",
  //                (void *)&interpret_0, Int64, 2, executionContextType,
  //                int32PointerType);
  // DefineFunction((char *)"interpret_1", (char *)__FILE__, "interpret_1",
  //                (void *)&interpret_1, Int64, 3, executionContextType,
  //                int32PointerType, stackElementType);
  // DefineFunction((char *)"interpret_2", (char *)__FILE__, "interpret_2",
  //                (void *)&interpret_2, Int64, 4, executionContextType,
  //                int32PointerType, stackElementType, stackElementType);
  // DefineFunction((char *)"interpret_3", (char *)__FILE__, "interpret_3",
  //                (void *)&interpret_3, Int64, 5, executionContextType,
  //                int32PointerType, stackElementType, stackElementType,
  //                stackElementType);
  // DefineFunction((char *)"bc_primitive", (char *)__FILE__, "bc_primitive",
  //                (void *)&bc_primitive, Int64, 2,
  //                executionContextPointerType, Int32);
}

#define QSTACK(b) (((VirtualMachineState *)(b)->vmState())->_stack)
#define QCOMMIT(b) \
  if (config_.operandStack) ((b)->vmState()->Commit(b));
#define QRELOAD(b) \
  if (config_.operandStack) ((b)->vmState()->Reload(b));
#define QRELOAD_DROP(b, toDrop) \
  if (config.operandStack) QSTACK(builder)->Drop(builder, toDrop);

long computeNumberOfBytecodes(const Instruction *program) {
  long result = 0;
  while (*program != NO_MORE_BYTECODES) {
    program++;
    result++;
  }
  return result;
}

bool MethodBuilder::inlineProgramIntoBuilder(
    int32_t programIndex, bool isTopLevel, TR::BytecodeBuilder *currentBuilder,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {
  bool success = true;
  maxInlineDepth--;
  const Instruction *program = functionSpec_.address;

  // Create a BytecodeBuilder for each Bytecode
  long numberOfBytecodes = computeNumberOfBytecodes(functionSpec_.address);
  std::vector<TR::BytecodeBuilder *> builderTable(numberOfBytecodes);
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

  if (isTopLevel) {
    // only initialize locals if top level, inlines will be stored into from
    // parent.
    if (config_.callStyle == CallStyle::passParameter) {
      int argsCount = functionSpec_.nargs;
      int regsCount = functionSpec_.nregs;
      for (int i = argsCount; i < argsCount + regsCount; i++) {
        storeVarIndex(builder, i,
                      builder->ConstInt64(0));  // init all temps to zero
      }
    } else {
      // arguments are &sp[-number_of_args]
      // temps are pushes onto the stack to &sp[number_of_temps]
      TR::IlValue *sp = builder->LoadIndirect("stack", "stackPointer",
                                              builder->ConstAddress(stack_));
      TR::IlValue *args =
          builder->IndexAt(stackElementPointerType, sp,
                           builder->ConstInt32(0 - functionSpec_.nargs));
      builder->Store("returnSP", args);
      TR::IlValue *newSP =
          builder->IndexAt(stackElementPointerType, sp,
                           builder->ConstInt32(functionSpec_.nregs));
      builder->StoreIndirect("stackType", "stackPointer",
                             builder->ConstAddress(stack_), newSP);
    }
  }

  // Create a BytecodeBuilder for each Bytecode
  for (int i = 0; i < numberOfBytecodes; i++) {
    ByteCode bc = Instructions::getByteCode(program[i]);
    if (!generateILForBytecode(builderTable, program, bc, i,
                               jumpToBuilderForInlinedReturn)) {
      success = false;
      break;
    }
  }

  maxInlineDepth++;
  return success;
}

bool MethodBuilder::buildIL() {
  if (config_.operandStack) {
    this->Store("stack", this->ConstAddress(stack_));
    OMR::VirtualMachineRegisterInStruct *stackTop =
        new OMR::VirtualMachineRegisterInStruct(this, "stackType", "stack",
                                                "stackPointer", "SP");
    OMR::VirtualMachineOperandStack *stack =
        new OMR::VirtualMachineOperandStack(this, 32, stackElementPointerType,
                                            stackTop, true, 0);
    VirtualMachineState *vms = new VirtualMachineState(stack, stackTop);
    setVMState(vms);
  } else {
    setVMState(new OMR::VirtualMachineState());
  }

  return inlineProgramIntoBuilder(topLevelProgramIndex, true);
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

  if (config_.callStyle == CallStyle::passParameter) {
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
  if (config_.callStyle == CallStyle::passParameter) {
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
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    const Instruction *program, ByteCode bytecode, long bytecodeIndex,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {
  TR::BytecodeBuilder *builder = bytecodeBuilderTable[bytecodeIndex];
  Instruction instruction = program[bytecodeIndex];
  // assert(bytecode == Instructions::getByteCode(instruction));

  if (NULL == builder) {
    printf("unexpected NULL BytecodeBuilder!\n");
    return false;
  }

  long numberOfBytecodes = computeNumberOfBytecodes(program);
  TR::BytecodeBuilder *nextBytecodeBuilder = nullptr;
  int32_t nextBytecodeIndex = bytecodeIndex + 1;
  if (nextBytecodeIndex < numberOfBytecodes) {
    nextBytecodeBuilder = bytecodeBuilderTable[nextBytecodeIndex];
  }

  bool handled = true;

  // if (context->debug >= 2) {
  //   if (jumpToBuilderForInlinedReturn) {
  //     printf("INLINED METHOD: skew %d return bc will jump to %p: ",
  //            firstArgumentIndex, jumpToBuilderForInlinedReturn);
  //   }
  //   printf("generating index=%d bc=%s(%d) param=%d \n", bytecodeIndex,
  //          ByteCodeName(bytecode), bytecode,
  //          getParameterFromInstruction(instruction));
  // }

  // if (context->debug == 2) {
  //   QCOMMIT(builder);
  //   builder->Call(
  //       "printVMState", 4, builder->ConstAddress(context),
  //       builder->ConstInt64(bytecodeIndex), builder->ConstInt64(bytecode),
  //       builder->ConstInt64(getParameterFromInstruction(instruction)));
  // }

  switch (bytecode) {
    case ByteCode::PUSH_FROM_VAR:
    //   push(builder,
    //        loadVarIndex(builder, getParameterFromInstruction(instruction)));
    //   if (nextBytecodeBuilder)
    //     builder->AddFallThroughBuilder(nextBytecodeBuilder);
    //   break;
    // case ByteCode::POP_INTO_VAR:
    //   storeVarIndex(builder, getParameterFromInstruction(instruction),
    //                 pop(builder));
    //   if (nextBytecodeBuilder)
    //     builder->AddFallThroughBuilder(nextBytecodeBuilder);
    //   break;
    // case ByteCode::FUNCTION_RETURN: {
    //   if (jumpToBuilderForInlinedReturn) {
    //     builder->Goto(jumpToBuilderForInlinedReturn);
    //   } else {
    //     auto result = pop(builder);
    //     if (!context->passParameters) {
    //       builder->StoreIndirect("executionContextType", "stackPointer",
    //                              builder->ConstAddress(context),
    //                              builder->Load("returnSP"));
    //     }
    //     builder->Return(result);
    //   }
    // } break;
    case ByteCode::DROP:
    //   drop(builder);
    //   if (nextBytecodeBuilder)
    //     builder->AddFallThroughBuilder(nextBytecodeBuilder);
    //   break;
    case ByteCode::JMP:
    //   handle_bc_jmp(builder, bytecodeBuilderTable, program, bytecodeIndex);
    //   break;
    case ByteCode::INT_JMP_EQ:
    //   handle_bc_jmp_eq(builder, bytecodeBuilderTable, program, bytecodeIndex,
    //                    nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_JMP_NEQ:
    //   handle_bc_jmp_neq(builder, bytecodeBuilderTable, program,
    //   bytecodeIndex,
    //                     nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_JMP_LT:
    //   handle_bc_jmp_lt(builder, bytecodeBuilderTable, program, bytecodeIndex,
    //                    nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_JMP_LE:
    //   handle_bc_jmp_le(builder, bytecodeBuilderTable, program, bytecodeIndex,
    //                    nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_JMP_GT:
    //   handle_bc_jmp_gt(builder, bytecodeBuilderTable, program, bytecodeIndex,
    //                    nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_JMP_GE:
    //   handle_bc_jmp_ge(builder, bytecodeBuilderTable, program, bytecodeIndex,
    //                    nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_SUB:
    //   handle_bc_sub(builder, nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_ADD:
    //   handle_bc_add(builder, nextBytecodeBuilder);
    //   break;
    case ByteCode::INT_PUSH_CONSTANT:
    // {
    //   int constvalue = getParameterFromInstruction(instruction);
    //   push(builder, builder->ConstInt64(constvalue));
    //   if (nextBytecodeBuilder)
    //     builder->AddFallThroughBuilder(nextBytecodeBuilder);
    // } break;
    case ByteCode::STR_PUSH_CONSTANT:
    // {
    //         int index = getParameterFromInstruction(instruction);
    //         push(builder,
    //         builder->ConstAddress(&context->stringTable[index]));
    //         builder->AddFallThroughBuilder(nextBytecodeBuilder);
    //       } break;
    case ByteCode::PRIMITIVE_CALL:
    // {
    //      int index = getParameterFromInstruction(instruction);
    //      push(builder,  builder->ConstAddress(&context->stringTable[index]));
    //      builder->AddFallThroughBuilder(nextBytecodeBuilder);
    //    } break;
    case ByteCode::FUNCTION_CALL:
    // {
    //   int callindex = getParameterFromInstruction(instruction);
    //   Instruction *tocall = context->functions[callindex].program;
    //   if (context->directCall) {
    //     int argsCount = progArgCount(*tocall);
    //     const char *interpretName[] = {"interpret_0", "interpret_1",
    //                                    "interpret_2", "interpret_3"};
    //     const char *nameToCall = interpretName[argsCount];
    //     bool interp = true;
    //     if (tocall == program ||
    //         context->functions[callindex].jitAddress != 0) {
    //       nameToCall = context->functions[callindex].name;
    //       interp = false;
    //     }
    //     if (context->passParameters) {
    //       if (maxInlineDepth >= 0 && !interp) {  // && tocall == program) {
    //         int32_t save = firstArgumentIndex;
    //         int32_t skipLocals =
    //             progArgCount(*program) + progTmpCount(*program);
    //         int32_t spaceNeeded = progArgCount(*tocall) +
    //         progTmpCount(*tocall); firstArgumentIndex += skipLocals;
    //         // no need to define locals here the outer program registered all
    //         // locals
    //         // it means some locals will be reused which will affect liveness
    //         of
    //         // a
    //         // variable
    //         if ((firstArgumentIndex + spaceNeeded) < MAX_ARGS_TEMPS_AVAIL) {
    //           // printf("INLINING RECURSION ONLY  old skew %d, new skew =
    //           %d\n",
    //           // save, firstArgumentIndex);
    //           int storeInto = progArgCount(*tocall);
    //           while (storeInto-- > 0) {
    //             // printf("Storing temp %d into dest variable \n",
    //             storeInto); storeVarIndex(builder, storeInto,
    //                           pop(builder));  // firstArgumentIndex is added
    //                           in
    //                                           // storeVarIndex
    //           }
    //           bool result = inlineProgramIntoBuilder(callindex, false,
    //           builder,
    //                                                  nextBytecodeBuilder);
    //           if (!result) {
    //             printf("Failed inlineProgramIntoBuilder\n");
    //             return result;
    //           }
    //           // printf("SETTING SKEW BACK from %d to %d\n",
    //           firstArgumentIndex,
    //           // save);
    //           firstArgumentIndex = save;
    //           break;
    //         } else {
    //           printf("SKIP INLINE DUE TO EXCESSIVE TEMPS NEEDED\n");
    //         }
    //       }
    //       if (argsCount > 8) {
    //         printf("ERROR Need to add handlers for more parameters\n");
    //         break;
    //       }
    //       TR::IlValue *p[8];
    //       memset(p, 0, sizeof(p));
    //       int popInto = argsCount;
    //       while (popInto--) {
    //         p[popInto] = pop(builder);
    //       }
    //       if (interp) {
    //         TR::IlValue *result = builder->Call(
    //             nameToCall, 2 + argsCount, builder->ConstAddress(context),
    //             builder->ConstAddress(tocall), p[0], p[1], p[2], p[3], p[4],
    //             p[5], p[6], p[7]);
    //         push(builder, result);
    //       } else {
    //         TR::IlValue *result =
    //             builder->Call(nameToCall, argsCount, p[0], p[1], p[2], p[3],
    //                           p[4], p[5], p[6], p[7]);
    //         push(builder, result);
    //       }
    //     } else {
    //       TR::IlValue *result;
    //       QCOMMIT(builder);
    //       if (interp) {
    //         result =
    //             builder->Call(nameToCall, 2, builder->ConstAddress(context),
    //                           builder->ConstAddress(tocall));
    //       } else {
    //         result = builder->Call(nameToCall, 0);
    //       }
    //       QRELOAD_DROP(builder, progArgCount(*tocall));
    //       push(builder, result);
    //     }
    //   } else {
    //     // only use interpreter to dispatch the calls
    //     QCOMMIT(builder);
    //     TR::IlValue *result =
    //         builder->Call("interpret", 2, builder->ConstAddress(context),
    //                       builder->ConstAddress(tocall));
    //     QRELOAD_DROP(builder, progArgCount(*tocall));
    //     push(builder, result);
    //   }

    //   if (nextBytecodeBuilder)
    //     builder->AddFallThroughBuilder(nextBytecodeBuilder);

    // } break;
    default:
      // if (config_.debug >= 1) {
      //   std::cout << " genIlForByteCode Failed, unrecognized byte code :%d\n"
      //   << bytecode << std::endl;
      // }
      handled = false;
      break;
  }

  return handled;
}

/*************************************************
 * GENERATE CODE FOR BYTECODES
 *************************************************/

void MethodBuilder::handle_bc_jmp(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder **bytecodeBuilderTable,
                                  Instruction *program, long bytecodeIndex) {
  // Instruction instruction = program[bytecodeIndex];
  // StackElement delta = getParameterFromInstruction(instruction) + 1;
  // int next_bc_index = bytecodeIndex + delta;
  // TR::BytecodeBuilder *destBuilder = bytecodeBuilderTable[next_bc_index];
  // builder->Goto(destBuilder);
}

void MethodBuilder::handle_bc_jmp_eq(TR::BytecodeBuilder *builder,
                                     TR::BytecodeBuilder **bytecodeBuilderTable,
                                     Instruction *program, long bytecodeIndex,
                                     TR::BytecodeBuilder *nextBuilder) {
  // Instruction instruction = program[bytecodeIndex];
  // StackElement delta = getParameterFromInstruction(instruction) + 1;
  // int next_bc_index = bytecodeIndex + delta;
  // TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // builder->IfCmpEqual(jumpTo, left, right);
  // builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_neq(
    TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable,
    Instruction *program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  // Instruction instruction = program[bytecodeIndex];
  // StackElement delta = getParameterFromInstruction(instruction) + 1;
  // int next_bc_index = bytecodeIndex + delta;
  // TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // builder->IfCmpNotEqual(jumpTo, left, right);
  // builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_lt(TR::BytecodeBuilder *builder,
                                     TR::BytecodeBuilder **bytecodeBuilderTable,
                                     Instruction *program, long bytecodeIndex,
                                     TR::BytecodeBuilder *nextBuilder) {
  // Instruction instruction = program[bytecodeIndex];
  // StackElement delta = getParameterFromInstruction(instruction) + 1;
  // int next_bc_index = bytecodeIndex + delta;
  // TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // builder->IfCmpLessThan(jumpTo, left, right);
  // builder->AddFallThroughBuilder(nextBuilder);
}
void MethodBuilder::handle_bc_jmp_le(TR::BytecodeBuilder *builder,
                                     TR::BytecodeBuilder **bytecodeBuilderTable,
                                     Instruction *program, long bytecodeIndex,
                                     TR::BytecodeBuilder *nextBuilder) {
  // Instruction instruction = program[bytecodeIndex];

  // StackElement delta = getParameterFromInstruction(instruction) + 1;

  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // int next_bc_index = bytecodeIndex + delta;
  // TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];
  // left = builder->Sub(left, builder->ConstInt64(1));
  // builder->IfCmpGreaterThan(jumpTo, right, left);  // swap and do a
  // greaterthan builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_gt(TR::BytecodeBuilder *builder,
                                     TR::BytecodeBuilder **bytecodeBuilderTable,
                                     Instruction *program, long bytecodeIndex,
                                     TR::BytecodeBuilder *nextBuilder) {
  // Instruction instruction = program[bytecodeIndex];
  // StackElement delta = getParameterFromInstruction(instruction) + 1;
  // int next_bc_index = bytecodeIndex + delta;
  // TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // builder->IfCmpGreaterThan(jumpTo, left, right);
  // builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_ge(TR::BytecodeBuilder *builder,
                                     TR::BytecodeBuilder **bytecodeBuilderTable,
                                     Instruction *program, long bytecodeIndex,
                                     TR::BytecodeBuilder *nextBuilder) {
  // Instruction instruction = program[bytecodeIndex];
  // StackElement delta = getParameterFromInstruction(instruction) + 1;
  // int next_bc_index = bytecodeIndex + delta;
  // TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // // if (left >= right) jump is converted to if (left > (right-1) jump
  // right = builder->Sub(right, builder->ConstInt64(1));
  // builder->IfCmpGreaterThan(jumpTo, left, right);
  // builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_sub(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // push(builder, builder->Sub(left, right));
  // builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_add(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  // TR::IlValue *right = pop(builder);
  // TR::IlValue *left = pop(builder);

  // push(builder, builder->Add(left, right));
  // builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::drop(TR::BytecodeBuilder *builder) {
  // pop(builder);
}

void MethodBuilder::handle_bc_push_string(TR::BytecodeBuilder *builder,
                                          TR::BytecodeBuilder *nextBuilder) {}

TR::IlValue *MethodBuilder::pop(TR::BytecodeBuilder *builder) {
  // if (context.operandStack) {
  //   return QSTACK(builder)->Pop(builder);
  // } else {
  //   TR::IlValue *sp = builder->LoadIndirect(
  //       "executionContextType", "stackPointer",
  //       builder->ConstAddress(context));
  //   TR::IlValue *newSP =
  //       builder->IndexAt(stackElementPointerType, sp,
  //       builder->ConstInt32(-1));
  //   builder->StoreIndirect("executionContextType", "stackPointer",
  //                          builder->ConstAddress(context), newSP);
  //   return builder->LoadAt(stackElementPointerType, newSP);
  // }
  return builder->ConstInt32(0);
}

void MethodBuilder::push(TR::BytecodeBuilder *builder, TR::IlValue *value) {
  // if (config_.operandStack) {
  //   QSTACK(builder)->Push(builder, value);
  // } else {
  //   TR::IlValue *sp = builder->LoadIndirect(
  //       "executionContextType", "stackPointer",
  //       builder->ConstAddress(context));
  //   builder->StoreAt(builder->ConvertTo(stackElementPointerType, sp),
  //                    builder->ConvertTo(stackElementType, value));
  //   TR::IlValue *newSP =
  //       builder->IndexAt(stackElementPointerType, sp,
  //       builder->ConstInt32(1));
  //   builder->StoreIndirect("executionContextType", "stackPointer",
  //                          builder->ConstAddress(context), newSP);
  // }
}

}  // namespace b9
