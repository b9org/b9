#ifndef B9_JIT_INCL
#define B9_JIT_INCL

#include <b9/ExecutionContext.hpp>
#include <b9/VirtualMachine.hpp>
#include "b9/instructions.hpp"

#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include <Jit.hpp>
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include <vector>

namespace b9 {

class FunctionSpec;
class Stack;

/// Function not found exception.
struct CompilationException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/// A collection of basic, built in types.
class GlobalTypes {
 public:
  GlobalTypes(TR::TypeDictionary &td);
  TR::IlType *addressPtr;
  TR::IlType *int64Ptr;
  TR::IlType *int32Ptr;
  TR::IlType *int16Ptr;

  TR::IlType *stackElement;
  TR::IlType *stackElementPtr;
  TR::IlType *instruction;
  TR::IlType *InstructionPtr;

  TR::IlType *operandStack;
  TR::IlType *executionContext;
};

class Compiler {
 public:
  Compiler(VirtualMachine &virtualMachine, const Config &cfg);
  JitFunction generateCode(const std::size_t functionIndex);

  const GlobalTypes &globalTypes() const { return globalTypes_; }

  TR::TypeDictionary &typeDictionary() { return typeDictionary_; }

  const TR::TypeDictionary &typeDictionary() const { return typeDictionary_; }

 private:
  TR::TypeDictionary typeDictionary_;
  const GlobalTypes globalTypes_;
  VirtualMachine &virtualMachine_;
  const Config &cfg_;
};

class MethodBuilder : public TR::MethodBuilder {
 public:
  MethodBuilder(Compiler &compiler, const std::size_t functionIndex);

  virtual bool buildIL();

 private:
  void defineFunctions();
  void defineLocals(std::size_t nargs);
  void defineParameters(std::size_t nargs);

  void createBuilderForBytecode(TR::BytecodeBuilder **bytecodeBuilderTable,
                                ByteCode bytecode, int64_t bytecodeIndex);
  bool generateILForBytecode(
      const std::size_t functionIndex,
      std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
      const Instruction *program, ByteCode bytecode, long bytecodeIndex,
      TR::BytecodeBuilder *jumpToBuilderForInlinedReturn);

  bool inlineProgramIntoBuilder(
      const std::size_t functionIndex, bool isTopLevel,
      TR::BytecodeBuilder *currentBuilder = 0,
      TR::BytecodeBuilder *jumpToBuilderForInlinedReturn = 0);

  TR::IlValue *pop(TR::BytecodeBuilder *builder);
  void push(TR::BytecodeBuilder *builder, TR::IlValue *value);
  void drop(TR::BytecodeBuilder *builder);

  TR::IlValue *loadVarIndex(TR::IlBuilder *builder, int varindex);
  void storeVarIndex(TR::IlBuilder *builder, int varindex, TR::IlValue *value);

  void handle_bc_push_constant(TR::BytecodeBuilder *builder,
                               TR::BytecodeBuilder *nextBuilder);
  void handle_bc_push_string(TR::BytecodeBuilder *builder,
                             TR::BytecodeBuilder *nextBuilder);
  void handle_bc_drop(TR::BytecodeBuilder *builder,
                      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_push_from_var(TR::BytecodeBuilder *builder,
                               TR::BytecodeBuilder *nextBuilder);
  void handle_bc_pop_into_var(TR::BytecodeBuilder *builder,
                              TR::BytecodeBuilder *nextBuilder);
  void handle_bc_sub(TR::BytecodeBuilder *builder,
                     TR::BytecodeBuilder *nextBuilder);
  void handle_bc_add(TR::BytecodeBuilder *builder,
                     TR::BytecodeBuilder *nextBuilder);
  // CASCON2017 - add handle_bc_mul and handle_bc_div here
  void handle_bc_not(TR::BytecodeBuilder *builder,
                     TR::BytecodeBuilder *nextBuilder);
  void handle_bc_call(TR::BytecodeBuilder *builder,
                      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp(TR::BytecodeBuilder *builder,
                     std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
                     const Instruction *program, long bytecodeIndex);
  void handle_bc_jmp_eq(TR::BytecodeBuilder *builder,
                        std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
                        const Instruction *program, long bytecodeIndex,
                        TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_neq(
      TR::BytecodeBuilder *builder,
      std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
      const Instruction *program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_lt(TR::BytecodeBuilder *builder,
                        std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
                        const Instruction *program, long bytecodeIndex,
                        TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_le(TR::BytecodeBuilder *builder,
                        std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
                        const Instruction *program, long bytecodeIndex,
                        TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_gt(TR::BytecodeBuilder *builder,
                        std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
                        const Instruction *program, long bytecodeIndex,
                        TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_ge(TR::BytecodeBuilder *builder,
                        std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
                        const Instruction *program, long bytecodeIndex,
                        TR::BytecodeBuilder *nextBuilder);

  const GlobalTypes &globalTypes() const { return compiler_.globalTypes(); }

  Compiler &compiler_;
  const std::size_t functionIndex_;
  int32_t maxInlineDepth;
  int32_t firstArgumentIndex;
};

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

}  // namespace b9

#endif
