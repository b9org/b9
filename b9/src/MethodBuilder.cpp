#include "b9/compiler/MethodBuilder.hpp"
#include "b9/ExecutionContext.hpp"
#include "b9/VirtualMachine.hpp"
#include "b9/compiler/Compiler.hpp"
#include "b9/instructions.hpp"

#include <OMR/Om/ValueBuilder.hpp>

#include <ilgen/VirtualMachineOperandStack.hpp>
#include <ilgen/VirtualMachineRegister.hpp>
#include <ilgen/VirtualMachineRegisterInStruct.hpp>

extern "C" {

void trace(b9::FunctionDef *function, b9::Instruction *instruction) {
  std::cerr << function->name << "@" << instruction << ": " << *instruction
            << std::endl;
}

void print_stack(b9::ExecutionContext *context) {
  printStack(std::cerr, context->stack());
}

void print_value(Om::Value v) { std::cerr << v << std::endl; }

void print_ptr(void *v) { std::cerr << v << std::endl; }

}  // extern "C"

namespace b9 {

MethodBuilder::MethodBuilder(VirtualMachine &virtualMachine,
                             const std::size_t functionIndex)
    : TR::MethodBuilder(&virtualMachine.compiler()->typeDictionary()),
      virtualMachine_(virtualMachine),
      cfg_(virtualMachine.config()),
      maxInlineDepth_(cfg_.maxInlineDepth),
      globalTypes_(virtualMachine.compiler()->globalTypes()),
      functionIndex_(functionIndex) {
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex);

  /// TODO: The __LINE__/__FILE__ stuff is 100% bogus, this is about as bad.
  DefineLine("<unknown");
  DefineFile(function->name.c_str());

  DefineName(function->name.c_str());

  DefineReturnType(globalTypes().stackElement);

  defineParams();

  defineLocals();

  defineFunctions();

  AllLocalsHaveBeenDefined();
}

static const std::string PARAM_STRING = "param";
static const std::string LOCAL_STRING = "local";

/// The first argument is always executionContext.
/// The remaining function arguments are only passed as native arguments in
/// PassParam mode.
void MethodBuilder::defineParams() {
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex_);
  if (cfg_.verbose) {
    std::cout << "Defining " << function->nparams << " parameters\n";
  }

  /// first argument is always the execution context
  DefineParameter("executionContext", globalTypes().executionContextPtr);

  /// In pass param, arguments are passed using C linkage. Otherwise, parameters
  /// are on the stack.
  if (cfg_.passParam) {
    params_.resize(function->nparams);
    for (int i = 0; i < function->nparams; i++) {
      params_[i] = PARAM_STRING + std::to_string(i);
      DefineParameter(params_[i].c_str(), globalTypes().stackElement);
    }
  }
}

void MethodBuilder::defineLocals() {
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex_);
  if (cfg_.verbose) {
    std::cout << "Defining " << function->nlocals << " locals\n";
  }

  // Pointer to the base of the frame.  Used for rolling back the stack pointer
  // on a return.
  DefineLocal("stackBase", globalTypes().stackElementPtr);

  // Pointer to the operand stack
  DefineLocal("stack", globalTypes().operandStackPtr);

  // Address of the current stack top
  DefineLocal("stackTop", globalTypes().stackElementPtr);

  locals_.resize(function->nlocals);

  for (std::size_t i = 0; i < function->nlocals; i++) {
    locals_[i] = LOCAL_STRING + std::to_string(i);
    DefineLocal(locals_[i].c_str(), globalTypes().stackElement);
  }
}

void MethodBuilder::defineFunctions() {
  int functionIndex = 0;
  while (functionIndex < virtualMachine_.getFunctionCount()) {
    if (virtualMachine_.getJitAddress(functionIndex) != nullptr) {
      auto function = virtualMachine_.getFunction(functionIndex);
      auto name = function->name.c_str();
      DefineFunction(name, (char *)__FILE__, name,
                     (void *)virtualMachine_.getJitAddress(functionIndex),
                     Int64, function->nparams, globalTypes().stackElement,
                     globalTypes().stackElement, globalTypes().stackElement,
                     globalTypes().stackElement, globalTypes().stackElement,
                     globalTypes().stackElement, globalTypes().stackElement,
                     globalTypes().stackElement);
    }
    functionIndex++;
  }

  DefineFunction((char *)"interpret", (char *)__FILE__, "interpret",
                 (void *)&interpret, Int64, 2,
                 globalTypes().executionContextPtr, globalTypes().size);
  DefineFunction((char *)"primitive_call", (char *)__FILE__, "primitive_call",
                 (void *)&primitive_call, NoType, 2,
                 globalTypes().executionContextPtr, Int32);
  DefineFunction((char *)"trace", (char *)__FILE__, "trace", (void *)&trace,
                 NoType, 2, globalTypes().addressPtr, globalTypes().addressPtr);
  DefineFunction((char *)"print_stack", (char *)__FILE__, "print_stack",
                 (void *)&print_stack, NoType, 1,
                 globalTypes().executionContextPtr);
  DefineFunction((char *)"print_value", (char *)__FILE__, "print_value",
                 (void *)&print_value, NoType, 1, globalTypes().stackElement);
  DefineFunction((char *)"print_ptr", (char *)__FILE__, "print_ptr",
                 (void *)&print_ptr, NoType, 1, globalTypes().addressPtr);
}

bool MethodBuilder::inlineProgramIntoBuilder(
    const std::size_t functionIndex, bool isTopLevel,
    TR::BytecodeBuilder *currentBuilder,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {
  bool success = true;
  maxInlineDepth_--;
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex);
  const Instruction *program = function->instructions.data();

  // Create a BytecodeBuilder for each Bytecode
  auto numberOfBytecodes = function->instructions.size();

  if (numberOfBytecodes == 0) {
    if (cfg_.verbose) {
      std::cerr << "unexpected EMPTY function body for " << function->name
                << std::endl;
    }
    return false;
  }

  if (cfg_.verbose)
    std::cout << "Creating " << numberOfBytecodes << " bytecode builders"
              << std::endl;

  // create the builders

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

  // Gen IL
  bool ok = true;

  for (std::size_t index = GetNextBytecodeFromWorklist(); index != -1;
       index = GetNextBytecodeFromWorklist()) {
    ok = generateILForBytecode(function, builderTable, index,
                               jumpToBuilderForInlinedReturn);
    if (!ok) break;
  }
  return ok;
}

bool MethodBuilder::buildIL() {
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex_);

  TR::IlValue *stack = StructFieldInstanceAddress(
      "b9::ExecutionContext", "stack_", Load("executionContext"));
  Store("stack", stack);

  TR::IlValue *stackTop = LoadIndirect("b9::OperandStack", "top_", stack);
  Store("stackTop", stackTop);

  if (cfg_.lazyVmState) {
    setVMState(new ModelState(this, globalTypes()));
  } else {
    setVMState(new ActiveState(this, globalTypes()));
  }

  /// When this function exits, we reset the stack top to the beginning of
  /// entry. The calling convention is callee-cleanup, so at exit we pop all
  /// the args off the operand stack.
  ///
  /// In the case of pass immediate, the arguments are not passed on the VM
  /// stack. The arguments are passed on the C stack as a part of a cdecl call.
  if (!cfg_.passParam) {
    TR::IlValue *stackBase = IndexAt(globalTypes().stackElementPtr, stackTop,
                                     ConstInt32(-function->nparams));
    Store("stackBase", stackBase);
  } else {
    Store("stackBase", stackTop);
  }

  return inlineProgramIntoBuilder(functionIndex_, true);
}

TR::IlValue *MethodBuilder::loadLocal(TR::IlBuilder *b, std::size_t index) {
  return b->Load(locals_[index].c_str());
}

void MethodBuilder::storeLocal(TR::IlBuilder *b, std::size_t index,
                               TR::IlValue *value) {
  b->Store(locals_[index].c_str(), value);
}

TR::IlValue *MethodBuilder::loadParam(TR::IlBuilder *b, std::size_t index) {
  if (cfg_.passParam) {
    return b->Load(params_[index].c_str());
  } else {
    TR::IlValue *args = b->Load("stackBase");
    TR::IlValue *address =
        b->IndexAt(globalTypes().stackElementPtr, args, b->ConstInt32(index));
    TR::IlValue *result = b->LoadAt(globalTypes().stackElementPtr, address);
    return result;
  }
}

void MethodBuilder::storeParam(TR::IlBuilder *b, std::size_t index,
                               TR::IlValue *value) {
  if (cfg_.passParam) {
    b->Store(params_[index].c_str(), value);
  } else {
    TR::IlValue *args = b->Load("stackBase");
    TR::IlValue *address =
        b->IndexAt(globalTypes().stackElementPtr, args, b->ConstInt32(index));
    b->StoreAt(address, value);
  }
}

bool MethodBuilder::generateILForBytecode(
    const FunctionDef *function,
    std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
    std::size_t instructionIndex,
    TR::BytecodeBuilder *jumpToBuilderForInlinedReturn) {
  TR::BytecodeBuilder *builder = bytecodeBuilderTable[instructionIndex];
  const std::vector<Instruction> &program = function->instructions;
  const Instruction instruction = program[instructionIndex];

  if (cfg_.verbose) {
    std::cout << "generating index=" << instructionIndex
              << " bc=" << instruction << std::endl;
  }

  if (nullptr == builder) {
    if (cfg_.verbose)
      std::cout << "unexpected NULL BytecodeBuilder!" << std::endl;
    return false;
  }

  TR::BytecodeBuilder *nextBytecodeBuilder = nullptr;

  if (instructionIndex + 1 <= program.size()) {
    nextBytecodeBuilder = bytecodeBuilderTable[instructionIndex + 1];
  }

  bool handled = true;

  if (cfg_.debug) {
    if (jumpToBuilderForInlinedReturn != nullptr) {
      std::cout << "INLINED METHOD: skew " << firstArgumentIndex
                << " return bc will jump to " << jumpToBuilderForInlinedReturn
                << ": ";
    }

    builder->Call("print_stack", 1, builder->Load("executionContext"));

    builder->Call(
        "trace", 2, builder->ConstAddress(function),
        builder->ConstAddress(&function->instructions[instructionIndex]));

    state(builder)->Commit(builder);
  }

  switch (instruction.opCode()) {
    case OpCode::PUSH_FROM_LOCAL:
      pushValue(builder, loadLocal(builder, instruction.immediate()));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case OpCode::POP_INTO_LOCAL:
      storeLocal(builder, instruction.immediate(), popValue(builder));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case OpCode::PUSH_FROM_PARAM:
      pushValue(builder, loadParam(builder, instruction.immediate()));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case OpCode::POP_INTO_PARAM:
      storeParam(builder, instruction.immediate(), popValue(builder));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case OpCode::FUNCTION_RETURN: {
      auto result = popValue(builder);
      TR::IlValue *stack = builder->StructFieldInstanceAddress(
          "b9::ExecutionContext", "stack_", builder->Load("executionContext"));
      builder->StoreIndirect("b9::OperandStack", "top_", stack,
                             builder->Load("stackBase"));
      builder->Return(result);
    } break;
    case OpCode::DUPLICATE: {
      auto x = popValue(builder);
      pushValue(builder, x);
      pushValue(builder, x);
      if (nextBytecodeBuilder) {
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      }
    } break;
    case OpCode::DROP:
      drop(builder);
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
      break;
    case OpCode::JMP:
      handle_bc_jmp(builder, bytecodeBuilderTable, program, instructionIndex,
                    nextBytecodeBuilder);
      break;
    case OpCode::JMP_EQ:
      handle_bc_jmp_eq(builder, bytecodeBuilderTable, program, instructionIndex,
                       nextBytecodeBuilder);
      break;
    case OpCode::JMP_NEQ:
      handle_bc_jmp_neq(builder, bytecodeBuilderTable, program,
                        instructionIndex, nextBytecodeBuilder);
      break;
    case OpCode::JMP_LT:
      handle_bc_jmp_lt(builder, bytecodeBuilderTable, program, instructionIndex,
                       nextBytecodeBuilder);
      break;
    case OpCode::JMP_LE:
      handle_bc_jmp_le(builder, bytecodeBuilderTable, program, instructionIndex,
                       nextBytecodeBuilder);
      break;
    case OpCode::JMP_GT:
      handle_bc_jmp_gt(builder, bytecodeBuilderTable, program, instructionIndex,
                       nextBytecodeBuilder);
      break;
    case OpCode::JMP_GE:
      handle_bc_jmp_ge(builder, bytecodeBuilderTable, program, instructionIndex,
                       nextBytecodeBuilder);
      break;
    case OpCode::INT_SUB:
      handle_bc_sub(builder, nextBytecodeBuilder);
      break;
    case OpCode::INT_ADD:
      handle_bc_add(builder, nextBytecodeBuilder);
      break;
    case OpCode::INT_MUL:
      handle_bc_mul(builder, nextBytecodeBuilder);
      break;
    case OpCode::INT_DIV:
      handle_bc_div(builder, nextBytecodeBuilder);
      break;
    case OpCode::INT_NOT:
      handle_bc_not(builder, nextBytecodeBuilder);
      break;
    case OpCode::INT_PUSH_CONSTANT: {
      int constvalue = instruction.immediate();
      /// TODO: box/unbox here.
      pushInt48(builder, builder->ConstInt64(constvalue));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case OpCode::STR_PUSH_CONSTANT: {
      int index = instruction.immediate();
      /// TODO: Box/unbox here.
      pushUint48(builder, builder->ConstInt64(index));
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case OpCode::PRIMITIVE_CALL: {
      state(builder)->Commit(builder);
      TR::IlValue *result =
          builder->Call("primitive_call", 2, builder->Load("executionContext"),
                        builder->ConstInt32(instruction.immediate()));
      state(builder)->Reload(builder);
      if (nextBytecodeBuilder)
        builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case OpCode::FUNCTION_CALL: {
      handle_bc_function_call(builder, nextBytecodeBuilder,
                              instruction.immediate());
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

void MethodBuilder::interpreterCall(TR::BytecodeBuilder *b,
                                    std::size_t target) {
  const auto &callee = virtualMachine_.module()->functions[target];

  if (cfg_.verbose) {
    std::cerr << "interpreterCall: " << callee.name << std::endl;
  }

  state(b)->Commit(b);
  TR::IlValue *result = b->Call("interpret", 2, b->Load("executionContext"),
                                b->ConstInt64(target));
  state(b)->adjust(b, -callee.nparams);
  state(b)->Reload(b);
  state(b)->pushValue(b, result);
}

void MethodBuilder::directCall(TR::BytecodeBuilder *b, std::size_t target) {
  const auto &callee = virtualMachine_.module()->functions[target];

  if (cfg_.verbose) {
    std::cout << "directCall: " << callee.name << std::endl;
  }

  assert(virtualMachine_.getJitAddress(target) || target == functionIndex_);

  state(b)->Commit(b);
  auto result = b->Call(callee.name.c_str(), 2, b->Load("executionContext"),
                        b->ConstInt64(target));
  state(b)->adjust(b, -callee.nparams);
  state(b)->Reload(b);
  state(b)->pushValue(b, result);
}

void MethodBuilder::passParamCall(TR::BytecodeBuilder *b, std::size_t target) {
  const auto &callee = virtualMachine_.module()->functions[target];

  if (cfg_.verbose) {
    std::cout << "passParamCall: " << callee.name << std::endl;
  }

  assert(virtualMachine_.getJitAddress(target) || target == functionIndex_);

  /// Pop the args for passing. Args are pushed left-to-right, so popping is
  /// right-to-left.
  std::vector<TR::IlValue *> params(callee.nparams + 1);
  for (std::size_t i = callee.nparams; i >= 1; --i) {
    std::cerr << "popping arg: " << callee.nparams - i << std::endl;
    params.at(i) = state(b)->popValue(b);
  }
  params.at(0) = b->Load("executionContext");

  auto result = b->Call(callee.name.c_str(), params.size(), params.data());
  state(b)->pushValue(b, result);
}

void MethodBuilder::handle_bc_function_call(TR::BytecodeBuilder *builder,
                                            TR::BytecodeBuilder *nextBuilder,
                                            std::size_t target) {
  bool interpret = cfg_.debug || (!virtualMachine_.getJitAddress(target) &&
                                  target != functionIndex_);

  if (interpret) {
    interpreterCall(builder, target);
  } else if (cfg_.passParam) {
    passParamCall(builder, target);
  } else if (cfg_.directCall) {
    directCall(builder, target);
  } else {
    interpreterCall(builder, target);
  }

  if (nextBuilder) builder->AddFallThroughBuilder(nextBuilder);
}

/*************************************************
 * GENERATE CODE FOR BYTECODES
 *************************************************/

void MethodBuilder::handle_bc_jmp(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *destBuilder = bytecodeBuilderTable[next_bc_index];
  builder->Goto(destBuilder);
}

void MethodBuilder::handle_bc_jmp_eq(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  builder->IfCmpEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_neq(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = popValue(builder);
  TR::IlValue *left = popValue(builder);

  builder->IfCmpNotEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_lt(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = popValue(builder);
  TR::IlValue *left = popValue(builder);

  builder->IfCmpLessThan(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_le(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  builder->IfCmpLessOrEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_gt(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  builder->IfCmpGreaterThan(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_jmp_ge(
    TR::BytecodeBuilder *builder,
    const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
    const std::vector<Instruction> &program, long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder) {
  Instruction instruction = program[bytecodeIndex];
  int delta = instruction.immediate() + 1;
  int next_bc_index = bytecodeIndex + delta;
  TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  builder->IfCmpGreaterOrEqual(jumpTo, left, right);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_sub(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  pushInt48(builder, builder->Sub(left, right));
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_add(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  pushInt48(builder, builder->Add(left, right));
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_mul(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  pushInt48(builder, builder->Mul(left, right));
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_div(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  TR::IlValue *right = popInt48(builder);
  TR::IlValue *left = popInt48(builder);

  pushInt48(builder, builder->Div(left, right));
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::handle_bc_not(TR::BytecodeBuilder *builder,
                                  TR::BytecodeBuilder *nextBuilder) {
  auto zero = builder->ConstInteger(globalTypes().stackElement, 0);
  auto value = popInt48(builder);
  auto result = builder->ConvertTo(globalTypes().stackElement,
                                   builder->EqualTo(value, zero));
  pushInt48(builder, result);
  builder->AddFallThroughBuilder(nextBuilder);
}

void MethodBuilder::drop(TR::BytecodeBuilder *builder, std::size_t n) {
  for (std::size_t i = 0; i < n; i++) popValue(builder);
}

/// Input is a Value.
/// state may model the push, or keep the vm updated.
void MethodBuilder::pushValue(TR::BytecodeBuilder *b, TR::IlValue *value) {
  return state(b)->pushValue(b, value);
}

/// output is an unboxed value.
/// State may model the pop, or keep the vm updated.
TR::IlValue *MethodBuilder::popValue(TR::BytecodeBuilder *b) {
  return state(b)->popValue(b);
}

/// input is an unboxed int48.
void MethodBuilder::pushInt48(TR::BytecodeBuilder *builder,
                              TR::IlValue *value) {
  pushValue(builder, OMR::Om::ValueBuilder::fromInt48(builder, value));
}

TR::IlValue *MethodBuilder::popInt48(TR::BytecodeBuilder *builder) {
  return OMR::Om::ValueBuilder::getInt48(builder, popValue(builder));
}

void MethodBuilder::pushUint48(TR::BytecodeBuilder *builder,
                               TR::IlValue *value) {
  pushValue(builder, OMR::Om::ValueBuilder::fromUint48(builder, value));
}

TR::IlValue *MethodBuilder::popUint48(TR::BytecodeBuilder *builder) {
  return OMR::Om::ValueBuilder::getUint48(builder, popValue(builder));
}

}  // namespace b9
