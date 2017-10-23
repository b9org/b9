#ifndef B9_JIT_INCL
#define B9_JIT_INCL

#include <b9.hpp>
#include <b9/core.hpp>

#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"

#include <vector>

namespace b9 {

class FunctionSpec;
class Stack;

/// Function not found exception.
struct CompilationException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class MethodBuilder : public TR::MethodBuilder {
 public:
  MethodBuilder(VirtualMachine *virtualMachine, TR::TypeDictionary *types,
                const Config &config, const FunctionSpec &functionSpec,
                Stack *stack);

  virtual bool buildIL();

 private:
  VirtualMachine *virtualMachine_;
  TR::TypeDictionary *types_;
  const Config &cfg_;
  const FunctionSpec &functionSpec_;
  Stack *stack_;
  int32_t maxInlineDepth;
  int32_t firstArgumentIndex;

  TR::IlType *stackType;
  TR::IlType *stackPointerType;
  TR::IlType *stackElementType;
  TR::IlType *stackElementPointerType;

  TR::IlType *addressPointerType;
  TR::IlType *int64PointerType;
  TR::IlType *int32PointerType;
  TR::IlType *int16PointerType;

  void defineFunctions();
  void defineLocals(std::size_t nargs);
  void defineParameters(std::size_t nargs);

  void createBuilderForBytecode(TR::BytecodeBuilder **bytecodeBuilderTable,
                                ByteCode bytecode, int64_t bytecodeIndex);
  bool generateILForBytecode(
      std::vector<TR::BytecodeBuilder *> bytecodeBuilderTable,
      const Instruction *program, ByteCode bytecode, long bytecodeIndex,
      TR::BytecodeBuilder *jumpToBuilderForInlinedReturn);

  bool inlineProgramIntoBuilder(
      bool isTopLevel, TR::BytecodeBuilder *currentBuilder = 0,
      TR::BytecodeBuilder *jumpToBuilderForInlinedReturn = 0);

  TR::IlValue *pop(TR::BytecodeBuilder *builder);
  void push(TR::BytecodeBuilder *builder, TR::IlValue *value);
  void drop(TR::BytecodeBuilder *builder);

  TR::IlValue *loadVarIndex(TR::BytecodeBuilder *builder, int varindex);
  void storeVarIndex(TR::BytecodeBuilder *builder, int varindex,
                     TR::IlValue *value);

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
};

class Compiler {
 public:
  Compiler(VirtualMachine *virtualMachine, const Config &cfg);
  uint8_t *generateCode(const FunctionSpec &functionSpec);

 private:
  TR::TypeDictionary types_;
  VirtualMachine *virtualMachine_;
  const Config &cfg_;
};

}  // namespace b9

#endif
