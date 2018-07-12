#if !defined(B9_METHODBBUILDER_HPP_)
#define B9_METHODBBUILDER_HPP_

#include "b9/VirtualMachine.hpp"
#include "b9/compiler/Compiler.hpp"
#include "b9/compiler/GlobalTypes.hpp"
#include "b9/compiler/State.hpp"
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

  void pushValue(TR::BytecodeBuilder *builder, TR::IlValue *value);

  TR::IlValue *popValue(TR::BytecodeBuilder *builder);

  void pushInt48(TR::BytecodeBuilder *builder, TR::IlValue *value);

  TR::IlValue *popInt48(TR::BytecodeBuilder *builder);

  void drop(TR::BytecodeBuilder *builder, std::size_t n = 1);

  TR::IlValue *loadVal(TR::IlBuilder *builder, int valIndex);

  void storeVal(TR::IlBuilder *builder, int valIndex, TR::IlValue *value);

  TR::IlValue *loadLocalIndex(TR::IlBuilder *builder, int localIndex);

  void storeLocalIndex(TR::IlBuilder *builder, int localIndex,
                       TR::IlValue *value);

  TR::IlValue *loadParamIndex(TR::IlBuilder *builder, int paramIndex);

  void storeParamIndex(TR::IlBuilder *builder, int paramIndex,
                       TR::IlValue *value);

  // Bytecode Handlers

  void handle_bc_push_constant(TR::BytecodeBuilder *builder,
                               TR::BytecodeBuilder *nextBuilder);
  void handle_bc_push_string(TR::BytecodeBuilder *builder,
                             TR::BytecodeBuilder *nextBuilder);
  void handle_bc_drop(TR::BytecodeBuilder *builder,
                      TR::BytecodeBuilder *nextBuilder);
  void handle_bc_push_from_local(TR::BytecodeBuilder *builder,
                                 TR::BytecodeBuilder *nextBuilder);
  void handle_bc_pop_into_local(TR::BytecodeBuilder *builder,
                                TR::BytecodeBuilder *nextBuilder);
  void handle_bc_push_from_param(TR::BytecodeBuilder *builder,
                                 TR::BytecodeBuilder *nextBuilder);
  void handle_bc_pop_into_param(TR::BytecodeBuilder *builder,
                                TR::BytecodeBuilder *nextBuilder);
  void handle_bc_sub(TR::BytecodeBuilder *builder,
                     TR::BytecodeBuilder *nextBuilder);
  void handle_bc_add(TR::BytecodeBuilder *builder,
                     TR::BytecodeBuilder *nextBuilder);
  void handle_bc_mul(TR::BytecodeBuilder *builder,
                     TR::BytecodeBuilder *nextBuilder);
  void handle_bc_div(TR::BytecodeBuilder *builder,
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
