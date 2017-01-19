

#ifndef B9_JIT_INCL
#define B9_JIT_INCL

namespace TR
{
class IlBuilder;
class BytecodeBuilder;
class TypeDictionary;
class IlType;
}

class B9Method : public TR::MethodBuilder
{
public:
    B9Method(TR::TypeDictionary *types, Instruction *program, ExecutionContext *context);
    virtual bool
    buildIL();

protected:
    TR::IlType *executionContextType;
    TR::IlType *executionContextPointerType;
    TR::IlType *stackElementType;
    TR::IlType *stackElementPointerType;
    TR::IlType *int64PointerType;
    TR::IlType *int32PointerType;
    TR::IlType *int16PointerType;

private:
    Instruction *program;
    ExecutionContext *context;

    void
    defineFunctions();
    void
    defineStructures(TR::TypeDictionary *types);
    void
    defineLocals();
    void
    defineParameters();

    void
    createBuilderForBytecode(TR::BytecodeBuilder **bytecodeBuilderTable, uint8_t bytecode, int64_t bytecodeIndex);
    bool
    generateILForBytecode(TR::BytecodeBuilder **bytecodeBuilderTable, uint8_t bytecode, long bytecodeIndex);

    TR::IlValue *
    pop(TR::BytecodeBuilder *builder);
    void
    push(TR::BytecodeBuilder *builder, TR::IlValue *value);
    void
    drop(TR::BytecodeBuilder *builder);

    TR::IlValue *
    loadVarIndex(TR::BytecodeBuilder *builder, int varindex);
    void
    storeVarIndex(TR::BytecodeBuilder *builder, int varindex, TR::IlValue *value);

    void
    handle_bc_push_constant(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_drop(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_push_from_var(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_pop_into_var(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_sub(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_add(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_call(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_jmp(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex);
    void
    handle_bc_jmp_eq(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_jmp_neq(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_jmp_lt(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_jmp_le(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_jmp_gt(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex, TR::BytecodeBuilder *nextBuilder);
    void
    handle_bc_jmp_ge(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex, TR::BytecodeBuilder *nextBuilder);

};

#endif
