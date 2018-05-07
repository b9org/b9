#if !defined(B9_METHODBBUILDER_HPP_)
#define B9_METHODBBUILDER_HPP_

#include "b9/VirtualMachine.hpp"
#include "b9/compiler/Compiler.hpp"
#include "b9/compiler/GlobalTypes.hpp"
#include "b9/compiler/VirtualMachineState.hpp"
#include "b9/instructions.hpp"

#include <Jit.hpp>
#include <ilgen/BytecodeBuilder.hpp>
#include <ilgen/MethodBuilder.hpp>
#include <ilgen/TypeDictionary.hpp>

namespace b9 {

class VirtualMachine;

class MethodBuilder : public TR::MethodBuilder {
 public:
  MethodBuilder(VirtualMachine &virtualMachine,
                const std::size_t functionIndex);

  virtual bool buildIL();

 private:
  void defineFunctions();
  void defineLocals();
  void defineParameters();

  /// For a single bytecode, generate the
  bool generateILForBytecode(
      const FunctionDef *function,
      std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
      std::size_t instructionIndex,
      TR::BytecodeBuilder *jumpToBuilderForInlinedReturn);

  bool inlineProgramIntoBuilder(
      const std::size_t functionIndex, bool isTopLevel,
      TR::BytecodeBuilder *currentBuilder = 0,
      TR::BytecodeBuilder *jumpToBuilderForInlinedReturn = 0);

  // Helpers

  TR::IlValue *pop(TR::BytecodeBuilder *builder);

  void push(TR::BytecodeBuilder *builder, TR::IlValue *value);

  void drop(TR::BytecodeBuilder *builder);

  TR::IlValue *loadVarIndex(TR::IlBuilder *builder, int varindex);

  void storeVarIndex(TR::IlBuilder *builder, int varindex, TR::IlValue *value);

  // Bytecode Handlers

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
  void handle_bc_not(TR::BytecodeBuilder *builder,
                     TR::BytecodeBuilder *nextBuilder);
  void handle_bc_call(TR::BytecodeBuilder *builder,
                      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp(
      TR::BytecodeBuilder *builder,
      const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
      const std::vector<Instruction> &program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_eq(
      TR::BytecodeBuilder *builder,
      const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
      const std::vector<Instruction> &program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_neq(
      TR::BytecodeBuilder *builder,
      const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
      const std::vector<Instruction> &program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_lt(
      TR::BytecodeBuilder *builder,
      const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
      const std::vector<Instruction> &program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_le(
      TR::BytecodeBuilder *builder,
      const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
      const std::vector<Instruction> &program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_gt(
      TR::BytecodeBuilder *builder,
      const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
      const std::vector<Instruction> &program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_jmp_ge(
      TR::BytecodeBuilder *builder,
      const std::vector<TR::BytecodeBuilder *> &bytecodeBuilderTable,
      const std::vector<Instruction> &program, long bytecodeIndex,
      TR::BytecodeBuilder *nextBuilder);

  const GlobalTypes &globalTypes() { return globalTypes_; }

  VirtualMachine &virtualMachine_;
  const GlobalTypes &globalTypes_;
  const Config &cfg_;
  const std::size_t functionIndex_;
  int32_t maxInlineDepth_;
  int32_t firstArgumentIndex = 0;
};

}  // namespace b9

#endif  // B9_METHODBBUILDER_HPP_
