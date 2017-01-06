
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <dlfcn.h>
#include <errno.h>
#include <assert.h>

#include "Jit.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include "b9.h"
#include "b9jit.hpp"
 
static void
printString(int64_t stringPointer)
{
#define PRINTSTRING_LINE LINETOSTR(__LINE__)
    char* strPtr = (char*)stringPointer;
    fprintf(stderr, "%s", strPtr);
}

static void
printInt64(int64_t value)
{
#define PRINTINT64_LINE LINETOSTR(__LINE__)
    fprintf(stderr, "%ld", value);
}

static void
printInt64Hex(int64_t value)
{
#define PRINTINT64HEX_LINE LINETOSTR(__LINE__)
    printf("%lx\n", value);
}

static void
newline()
{
#define NEWLINE_LINE LINETOSTR(__LINE__)
    fprintf(stderr, "\n");
}

static void
printstring(char* s)
{
    printf("PS: <%s> \n", s);
}

class B9VirtualMachineState : public OMR::VirtualMachineState {
public:
    B9VirtualMachineState()
        : OMR::VirtualMachineState()
        , _stack(NULL)
        , _stackTop(NULL)
    {
    }

    B9VirtualMachineState(OMR::VirtualMachineOperandStack* stack, OMR::VirtualMachineRegister* stackTop)
        : OMR::VirtualMachineState()
        , _stack(stack)
        , _stackTop(stackTop)
    {
    }

    virtual void Commit(TR::IlBuilder* b)
    {
        //printf("Begin B9VirtualMachineState Commit\n");
        _stack->Commit(b);
        _stackTop->Commit(b);
        //printf("End B9VirtualMachineState Commit\n");
    }

    virtual void Reload(TR::IlBuilder* b)
    {
        printf("Begin B9VirtualMachineState Reload\n");
        _stackTop->Reload(b);
        _stack->Reload(b); 
    }

    virtual VirtualMachineState* MakeCopy()
    {
        // printf("IN B9VirtualMachineState* MakeCopy\n");
        B9VirtualMachineState* newState = new B9VirtualMachineState();
        newState->_stack = (OMR::VirtualMachineOperandStack*)_stack->MakeCopy();
        newState->_stackTop = (OMR::VirtualMachineRegister*)_stackTop->MakeCopy();
        return newState;
    }

    virtual void MergeInto(VirtualMachineState* other, TR::IlBuilder* b)
    {
        // printf("IN B9VirtualMachineState* MergeInto\n");
        B9VirtualMachineState* otherState = (B9VirtualMachineState*)other;
        _stack->MergeInto(otherState->_stack, b);
        _stackTop->MergeInto(otherState->_stackTop, b);
    }

    OMR::VirtualMachineOperandStack* _stack;
    OMR::VirtualMachineRegister* _stackTop;
};

void b9_jit_init()
{
    initializeJit();
}

void generateCode(Instruction* program)
{
    TR::TypeDictionary types;
    B9Method methodBuilder(&types, program);
    uint8_t* entry = 0;
    printf("Start gen code\n");
    int rc = (*compileMethodBuilder)(&methodBuilder, &entry);
    if (0 == rc) {
            printf("Compiled success address = <%p>\n", entry);
            uint64_t *slotForJitAddress = (uint64_t *) &program[1];
            *slotForJitAddress = (uint64_t)entry;
    } else {
            printf("Failed to compile");
    }
     printf("Done gen code\n", entry);
}

B9Method::B9Method(TR::TypeDictionary* types, Instruction *program)
    : MethodBuilder(types)
    , program(program) 
{
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
 
    char* signature = "compiled code";
    char* methodName = (char*)malloc(strlen(signature) + 1);
    printf("Generating -> <%s>\n", signature);
    snprintf(methodName, sizeof(methodName), "%s", signature);

    DefineName(methodName);
    DefineReturnType(NoType);

    defineStructures(types);
    defineParameters();
    defineLocals();
    defineFunctions();

    AllLocalsHaveBeenDefined();
}

void B9Method::defineParameters()
{
    DefineParameter("context", p_b9_execution_context); 
    DefineParameter("program", pInt32);
}

void B9Method::defineLocals()
{
    DefineLocal("args", pInt16);
    DefineLocal("nargs", Int64);
    DefineLocal("tmps", Int64);
}

void B9Method::defineStructures(TR::TypeDictionary* types)
{
    pInt64 = types->PointerTo(Int64); 
    pInt32 = types->PointerTo(Int32);
    pInt16 = types->PointerTo(Int16);

    b9_execution_context = types->DefineStruct("b9_execution_context");
    types->DefineField( "b9_execution_context", "stack", pInt16, offsetof(struct ExecutionContext, stack));
    types->DefineField( "b9_execution_context", "stackPointer", pInt16, offsetof(struct ExecutionContext, stackPointer));
    types->DefineField( "b9_execution_context", "functions", pInt64, offsetof(struct ExecutionContext, functions));
    types->CloseStruct("b9_execution_context");

    p_b9_execution_context = types->PointerTo(b9_execution_context);
}

long getargs (Instruction p)
{
    return progArgCount(p);
}

long gettemps (Instruction p)
{
    return progTmpCount(p);
}
  
void B9Method::defineFunctions()
{
    DefineFunction((char*)"printString", (char*)__FILE__, (char*)PRINTSTRING_LINE, (void*)&printString, NoType, 1, Int64);
    DefineFunction((char*)"printInt64", (char*)__FILE__, (char*)PRINTINT64_LINE, (void*)&printInt64, NoType, 1, Int64);
    DefineFunction((char*)"printInt64Hex", (char*)__FILE__, (char*)PRINTINT64HEX_LINE, (void*)&printInt64Hex, NoType, 1, Int64);

    DefineFunction((char*)"newline", (char*)__FILE__, (char*)NEWLINE_LINE, (void*)&newline, NoType, 0);
    DefineFunction((char*)"printstring", (char*)__FILE__, (char*)NEWLINE_LINE, (void*)&printstring, NoType, 1, Int64);

   DefineFunction((char*)"getargs", (char*)__FILE__, (char*)NEWLINE_LINE, (void*)&getargs, Int64, 1, Int64);
   DefineFunction((char*)"gettemps", (char*)__FILE__, (char*)NEWLINE_LINE, (void*)&gettemps, Int64, 1, Int64);

    // void bc_call(ExecutionContext* context, uint16_t value);

    DefineFunction((char*)"printStack", (char*)__FILE__, "printStack", (void*)&b9PrintStack, NoType, 1, Int64);
    // DefineFunction((char*)"bc_call", (char*)__FILE__, "bc_call", (void*)&bc_call, Int64, 2, Int64, Int64);
    DefineFunction((char*)"interpret", (char*)__FILE__, "interpret", (void*)&interpret, Int64, 2, Int64, Int64);
}

bool hackVMState = false;

#define QSTACK(b) (((B9VirtualMachineState*)(b)->vmState())->_stack)
#define QCOMMIT(b)                   \
    if (hackVMState) {  \
        ((b)->vmState()->Commit(b)); \
    }
#define QRELOAD(b)                   \
    if (hackVMState) {  \
        ((b)->vmState()->Reload(b)); \
    }
 
  char *b9_bytecodename(int bc) {
      return "FIX b9_bytecodename";
  }

void B9Method::createBuilderForBytecode(TR::BytecodeBuilder** bytecodeBuilderTable, uint8_t bytecode, int64_t bytecodeIndex)
{
    TR::BytecodeBuilder* newBuilder = OrphanBytecodeBuilder(bytecodeIndex, (char*)b9_bytecodename(bytecode));
    printf("Created bytecodebuilder index=%d bc=%d %p\n", bytecodeIndex, bytecode, newBuilder);
    bytecodeBuilderTable[bytecodeIndex] = newBuilder;
}

long computeNumberOfBytecodes(Instruction *program)
{
    long result = METHOD_FIRST_BC_OFFSET;
    program += METHOD_FIRST_BC_OFFSET;
    while(*program != NO_MORE_BYTECODES) {
        program++;
        result++;
    }
    // printf("bytecodeCount = %d\n", result);
    return result;
}

bool B9Method::buildIL()
{
    TR::BytecodeBuilder** bytecodeBuilderTable = nullptr;
    bool success = true;

    OMR::VirtualMachineRegisterInStruct* stackTop
        = new OMR::VirtualMachineRegisterInStruct(this, "b9_execution_context", "context", "stackPointer", "SP");
    OMR::VirtualMachineOperandStack* stack = new OMR::VirtualMachineOperandStack(this, 32, pInt64, stackTop);
    B9VirtualMachineState* vms = new B9VirtualMachineState(stack, stackTop);
    setVMState(vms);

    long numberOfBytecodes = computeNumberOfBytecodes(program);

    long tableSize = sizeof(TR::BytecodeBuilder*) * numberOfBytecodes;
    bytecodeBuilderTable = (TR::BytecodeBuilder**)malloc(tableSize);
    if (NULL == bytecodeBuilderTable) {
        return false;
    }
    memset(bytecodeBuilderTable, 0, tableSize);

long i;

    i = METHOD_FIRST_BC_OFFSET;
    while (i < numberOfBytecodes) {
        ByteCode bc = getByteCodeFromInstruction(program[i]);
        createBuilderForBytecode(bytecodeBuilderTable, bc, i);
        i += 1;
    }
    TR::BytecodeBuilder* builder = bytecodeBuilderTable[METHOD_FIRST_BC_OFFSET];
    printf("builder %p\n", builder);
    AppendBuilder(builder);
    
    builder->Call("printstring", 1, builder->ConstString("hi jon"));
    TR::IlValue* prog = builder->Load("program");
    TR::IlValue* sp = builder->LoadIndirect("b9_execution_context", "stackPointer", builder->Load("context"));

    TR::IlValue* nargs = builder->ConstInt32(progArgCount(*program) * sizeof(uint16_t));
    TR::IlValue* tmps = builder->ConstInt32(progTmpCount(*program));
    TR::IlValue* args = builder->Sub(sp, nargs);

    builder->Store("args", args);

    builder->Call("printstring", 1, builder->ConstString("!!buildIl sp,nargs,tmps,args="));
    builder->Call("printInt64Hex", 1, sp);
    builder->Call("printInt64Hex", 1, nargs);
    builder->Call("printInt64Hex", 1, tmps);
    builder->Call("printInt64Hex", 1, args);

    sp = Add(sp, tmps);

    i = METHOD_FIRST_BC_OFFSET;
    while (i < numberOfBytecodes) {
            Instruction bc = getByteCodeFromInstruction(program[i]);
            if (!generateILForBytecode(bytecodeBuilderTable, bc, i)) {
                success = false;
                break;
            }

        i += 1;
    }

    free((void*)bytecodeBuilderTable);
    return success;
}

TR::IlValue *B9Method::loadVarIndex(TR::BytecodeBuilder *builder, int varindex) {
  TR::IlValue *args = builder->Load("args");

  TR::IlValue *address =
      builder->IndexAt(pInt16, args, builder->ConstInt32(varindex));

  builder->Call("printstring", 1, builder->ConstString("loadVarIndex args="));
  builder->Call("printInt64Hex", 1, args);
  builder->Call("printstring", 1, builder->ConstString(" varindex="));
  builder->Call("printInt64Hex", 1, builder->ConstInt32(varindex));
  builder->Call("printstring", 1, builder->ConstString(" address="));
  builder->Call("printInt64Hex", 1, address);

  TR::IlValue *result = builder->LoadAt(pInt16, address);

  builder->Call("printstring", 1, builder->ConstString(" result="));
  builder->Call("printInt64Hex", 1, result);

  builder->Call("printStack", 1, builder->Load("context"));

    result = builder->ConvertTo(Int64, result);
      builder->Call("printstring", 1, builder->ConstString(" result="));
  builder->Call("printInt64Hex", 1, result);
  return result;
}

void B9Method::storeVarIndex(TR::BytecodeBuilder* builder, int varindex, TR::IlValue* value)
{
    TR::IlValue* args = builder->Load("args");

    TR::IlValue* address = builder->IndexAt(pInt16,
        args,
        builder->ConstInt32(varindex));

    builder->StoreAt(address, value);
}

bool B9Method::generateILForBytecode(TR::BytecodeBuilder** bytecodeBuilderTable,
    uint8_t bytecode, long bytecodeIndex)
{
    TR::BytecodeBuilder* builder = bytecodeBuilderTable[bytecodeIndex];

    TR::IlValue* args = builder->Load("args");
    Instruction instruction = program[bytecodeIndex];

    assert(bytecode == getByteCodeFromInstruction(instruction));

    printf("generateILForBytecode builder %lx\n", (uint64_t)builder);

    if (NULL == builder) {
        printf("unexpected NULL BytecodeBuilder!\n");
        return false;
    }

    long numberOfBytecodes = computeNumberOfBytecodes(program);
    TR::BytecodeBuilder* nextBytecodeBuilder = nullptr;
    long nextBytecodeIndex = bytecodeIndex + 1;
    if (nextBytecodeIndex < numberOfBytecodes) {
        nextBytecodeBuilder = bytecodeBuilderTable[nextBytecodeIndex];
    }

    bool handled = true;

    printf("generating bytecode %d byteCodeIndex %d \n", bytecode, bytecodeIndex);

    switch (bytecode) {
        
    case PUSH_FROM_VAR:
        push(builder, loadVarIndex(builder, getParameterFromInstruction(instruction)));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case POP_INTO_VAR:
        storeVarIndex(builder, getParameterFromInstruction(instruction), pop(builder));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case RETURN:
    {
        auto result = pop(builder);
        builder->StoreIndirect("b9_execution_context", "stackPointer", builder->Load("context"), args);
        builder->Return(result);
    }
        break;
    case DROP:
        drop(builder);
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case JMP:
        handle_bc_jmp(builder, bytecodeBuilderTable, bytecodeIndex);
        break;
    // case JMPLE:
    //     handle_bc_jmp_le(builder, bytecodeBuilderTable, bytecodeIndex, prevBytecodeIndex, nextBytecodeBuilder);
    //     break;
    case SUB:
        handle_bc_sub(builder, nextBytecodeBuilder);
        break;
    case ADD:
        handle_bc_add(builder, nextBytecodeBuilder);
        break;
    case PUSH_CONSTANT: {
        int constvalue = getParameterFromInstruction(instruction);
        printf("generating push_constant %d\n", constvalue);
        push(builder, builder->ConstInt16(constvalue));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case CALL: {
        int callindex = getParameterFromInstruction(instruction);
        TR::IlValue* functions = builder->LoadIndirect("b9_execution_context", "functions", builder->Load("context"));
        TR::IlValue *address = builder->IndexAt(pInt64, functions, builder->ConstInt32(callindex));
        TR::IlValue* result = builder->Call("interpret", 2, builder->Load("context"), builder->LoadAt(pInt64, address));
        push(builder, result);

        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);

    } break;
    default:
        handled = false;
        break;
    }

    return handled;
}

/*************************************************
 * GENERATE CODE FOR BYTECODES
 *************************************************/
 

void B9Method::handle_bc_jmp(TR::BytecodeBuilder* builder, TR::BytecodeBuilder** bytecodeBuilderTable, long bytecodeIndex)
{
    Instruction instruction = program[bytecodeIndex];

    int delta = getParameterFromInstruction(instruction);
    int next_bc_index = bytecodeIndex + delta;

    TR::BytecodeBuilder* destBuilder = bytecodeBuilderTable[next_bc_index];
    builder->Goto(destBuilder);
}
    

static int genBCIndex = 1024;
void B9Method::handle_bc_jmp_le(TR::BytecodeBuilder* builder,
    TR::BytecodeBuilder** bytecodeBuilderTable,
    long bytecodeIndex,
    int prevBytecodeIndex,
    TR::BytecodeBuilder* nextBuilder)
{
        Instruction instruction = program[bytecodeIndex];

        int delta = getParameterFromInstruction(instruction);

        TR::IlValue* right = pop(builder);
        TR::IlValue* left = pop(builder);

        int next_bc_index = bytecodeIndex + delta;
        TR::BytecodeBuilder* thenPath = bytecodeBuilderTable[next_bc_index];

        builder->IfCmpLessOrEqual(thenPath, left, right);
        builder->AddFallThroughBuilder(nextBuilder);
}

void B9Method::handle_bc_sub(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* nextBuilder)
{
    TR::IlValue* right = pop(builder);
    TR::IlValue* left = pop(builder);

    push(builder, builder->Sub(left, right));
    builder->AddFallThroughBuilder(nextBuilder);
}

void B9Method::handle_bc_add(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* nextBuilder)
{
    TR::IlValue* right = pop(builder);
    TR::IlValue* left = pop(builder);

    push(builder, builder->Add(left, right));
    builder->AddFallThroughBuilder(nextBuilder);
}

void B9Method::drop(TR::BytecodeBuilder* builder)
{ 
    pop(builder);
}

TR::IlValue* B9Method::pop(TR::BytecodeBuilder* builder)
{ 

    if (hackVMState) {
        //printf("QPOP <%s> stack %d\n", __func__, stackLevel);
        return QSTACK(builder)->Pop(builder);
    } else {
        // return *--sp

                TR::IlValue* sp = builder->LoadIndirect("b9_execution_context", "stackPointer", builder->Load("context"));
 TR::IlValue* newSP = builder->Sub(sp, builder->ConstInt64(2));
 builder->StoreIndirect( "b9_execution_context", "stackPointer", builder->Load("context"), newSP);

  TR::IlValue* value = builder->LoadAt(pInt16, newSP);
        return value;
    }
}

void B9Method::push(TR::BytecodeBuilder* builder, TR::IlValue* value)
{ 
    if (hackVMState) {
        // printf("QPUSH <%s> stack %d\n", __func__, stackLevel);
        return QSTACK(builder)->Push(builder, value);
    } else {

        // *vm->sp++ = value
        TR::IlValue* sp = builder->LoadIndirect("b9_execution_context", "stackPointer", builder->Load("context"));
        
        builder->Call("printstring", 1, builder->ConstString("IN PUSH: sp="));
    builder->Call("printInt64Hex", 1, sp);

        builder->StoreAt(builder->ConvertTo(pInt16, sp), builder->ConvertTo(Int16, value));

        TR::IlValue* newSP = builder->Add(sp, builder->ConstInt64(2));
        builder->StoreIndirect( "b9_execution_context", "stackPointer", builder->Load("context"), newSP);
    
        builder->Call("printstring", 1, builder->ConstString("AFTER PUSH sp="));
        builder->Call("printInt64Hex", 1, newSP);
        builder->Call("printStack", 1, builder->Load("context"));

    }
} 
