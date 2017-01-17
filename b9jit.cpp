
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Jit.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include "b9.h"
#include "b9jit.hpp"

const char*
b9_bytecodename(ByteCode bc)
{
    if (bc == PUSH_CONSTANT)
        return "PUSH_CONSTANT";
    if (bc == DROP)
        return "DROP";
    if (bc == PUSH_FROM_VAR)
        return "PUSH_FROM_VAR";
    if (bc == POP_INTO_VAR)
        return "POP_INTO_VAR";
    if (bc == SUB)
        return "SUB";
    if (bc == ADD)
        return "ADD";
    if (bc == CALL)
        return "CALL";
    if (bc == RETURN)
        return "RETURN";
    if (bc == JMPLE)
        return "JMPLE";
    if (bc == JMP)
        return "JMP";
    return "unknown bc";
}

void printVMState(ExecutionContext* context, int64_t pc, ByteCode bytecode, Parameter param)
{
    printf("Executing at pc %lld, bc is (%d, %s), param is (%d)\n", pc, bytecode, b9_bytecodename(bytecode), param);
    b9PrintStack(context);
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

    virtual void
    Commit(TR::IlBuilder* b)
    {
        _stack->Commit(b);
        _stackTop->Commit(b);
    }

    virtual void
    Reload(TR::IlBuilder* b)
    {
        _stackTop->Reload(b);
        _stack->Reload(b);
    }

    virtual VirtualMachineState*
    MakeCopy()
    {
        B9VirtualMachineState* newState = new B9VirtualMachineState();
        newState->_stack = (OMR::VirtualMachineOperandStack*)_stack->MakeCopy();
        newState->_stackTop = (OMR::VirtualMachineRegister*)_stackTop->MakeCopy();
        return newState;
    }

    virtual void
    MergeInto(VirtualMachineState* other, TR::IlBuilder* b)
    {
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

void generateCode(Instruction* program, ExecutionContext* context)
{
    TR::TypeDictionary types;
    // todo pass in context->functions
    B9Method methodBuilder(&types, program, context);
    uint8_t* entry = 0;
    // printf("Start gen code\n");
    int rc = (*compileMethodBuilder)(&methodBuilder, &entry);
    if (0 == rc) {
        // printf("Compiled success address = <%p>\n", entry);
        uint64_t* slotForJitAddress = (uint64_t*)&program[1];
        *slotForJitAddress = (uint64_t)entry;
    } else {
        printf("Failed to compile");
    }
    // printf("Done gen code\n");
}

B9Method::B9Method(TR::TypeDictionary* types, Instruction* program, ExecutionContext* context)
    : MethodBuilder(types)
    , program(program)
    , context(context)
{
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);

    const char* signature = "jit";
    int len = strlen(signature) + 16;
    char* methodName = (char*)malloc(len);
    snprintf(methodName, len, "%s_%p", signature, program);
    // printf("Generating from %p -> <%s>\n", program, methodName);

    DefineName(methodName);
    if (sizeof(StackElement) == 2) {
        stackElementType = Int16;
        stackElementPointerType = types->PointerTo(stackElementType);
    }
    if (sizeof(StackElement) == 4) {
        stackElementType = Int32;
        stackElementPointerType = types->PointerTo(stackElementType);
    }
    if (sizeof(StackElement) == 8) {
        stackElementType = Int64;
        stackElementPointerType = types->PointerTo(stackElementType);
    }

    DefineReturnType(stackElementType);

    defineStructures(types);
    defineParameters();
    defineLocals();
    defineFunctions();

    AllLocalsHaveBeenDefined();
}

static const char* argsAndTempNames[] = { "arg0", "arg1", "arg2", "arg3", "arg4", "arg5", "arg6", "arg7", "arg8", "arg9" };

void B9Method::defineParameters()
{
    DefineParameter("context", executionContextType);
    DefineParameter("program", int32PointerType);
    if (context->passParameters) {
        int argsCount = progArgCount(*program);
        for (int i = 0; i < argsCount; i++) {
            DefineParameter(argsAndTempNames[i], stackElementType);
        }
    }
}

void B9Method::defineLocals()
{
    int argsCount = progArgCount(*program);
    int tempCount = progTmpCount(*program);
    if (context->passParameters) {
        for (int i = argsCount; i < argsCount + tempCount; i++) {
            DefineLocal(argsAndTempNames[i], stackElementType);
        }
    } else {
        DefineLocal("returnSP", Int64);
    }
}

void B9Method::defineStructures(TR::TypeDictionary* types)
{
    int64PointerType = types->PointerTo(Int64);
    int32PointerType = types->PointerTo(Int32);
    int16PointerType = types->PointerTo(Int16);

    executionContextType = types->DefineStruct("executionContextType");
    types->DefineField("executionContextType", "stack", stackElementPointerType, offsetof(struct ExecutionContext, stack));
    types->DefineField("executionContextType", "stackPointer", stackElementPointerType, offsetof(struct ExecutionContext, stackPointer));
    types->DefineField("executionContextType", "functions", int64PointerType, offsetof(struct ExecutionContext, functions));
    types->CloseStruct("executionContextType");

    executionContextPointerType = types->PointerTo(executionContextType);
}

void B9Method::defineFunctions()
{
    DefineFunction((char*)"printVMState", (char*)__FILE__, "printVMState", (void*)&printVMState, NoType, 4, Int64, Int64, Int64, Int64);
    DefineFunction((char*)"printStack", (char*)__FILE__, "printStack", (void*)&b9PrintStack, NoType, 1, Int64);
    DefineFunction((char*)"interpret", (char*)__FILE__, "interpret", (void*)&interpret, Int64, 2,
        executionContextType, int32PointerType);
}

#define QSTACK(b)  (((B9VirtualMachineState*)(b)->vmState())->_stack)
#define QCOMMIT(b) if (context->operandStack) ((b)->vmState()->Commit(b));
#define QRELOAD(b) if (context->operandStack) ((b)->vmState()->Reload(b));
#define QRELOAD_DROP(b, toDrop) if (context->operandStack)  QSTACK(builder)->Drop(builder, toDrop);

void B9Method::createBuilderForBytecode(TR::BytecodeBuilder** bytecodeBuilderTable, uint8_t bytecode, int64_t bytecodeIndex)
{
    TR::BytecodeBuilder* newBuilder = OrphanBytecodeBuilder(bytecodeIndex, (char*)b9_bytecodename(bytecode));
    //printf("Created bytecodebuilder index=%d bc=%d param=%d %p\n", bytecodeIndex, bytecode, getParameterFromInstruction(program[bytecodeIndex]), newBuilder);
    bytecodeBuilderTable[bytecodeIndex] = newBuilder;
}

long computeNumberOfBytecodes(Instruction* program)
{
    long result = METHOD_FIRST_BC_OFFSET;
    program += METHOD_FIRST_BC_OFFSET;
    while (*program != NO_MORE_BYTECODES) {
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

    OMR::VirtualMachineRegisterInStruct* stackTop = new OMR::VirtualMachineRegisterInStruct(this, "executionContextType", "context", "stackPointer", "SP");
    OMR::VirtualMachineOperandStack* stack = new OMR::VirtualMachineOperandStack(this, 32, stackElementPointerType, stackTop);
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
    //printf("builder %p\n", builder);
    AppendBuilder(builder);

    if (context->passParameters) {
        int argsCount = progArgCount(*program);
        int tempCount = progTmpCount(*program);
        for (int i = argsCount; i < argsCount + tempCount; i++) {
            storeVarIndex(builder, i, builder->ConstInt64(0)); // init all temps to zero
        }
    } else {
        // arguments are &sp[-number_of_args]
        // temps are pushes onto the stack to &sp[number_of_temps]
        TR::IlValue* sp = builder->LoadIndirect("executionContextType", "stackPointer", builder->Load("context"));
        TR::IlValue* args = builder->IndexAt(stackElementPointerType, sp, builder->ConstInt32(0 - progArgCount(*program)));
        builder->Store("returnSP", args);
        TR::IlValue* newSP = builder->IndexAt(stackElementPointerType, sp, builder->ConstInt32(progTmpCount(*program)));
        builder->StoreIndirect("executionContextType", "stackPointer", builder->Load("context"), newSP);
    }

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

TR::IlValue*
B9Method::loadVarIndex(TR::BytecodeBuilder* builder, int varindex)
{
    if (context->passParameters) {
        return builder->Load(argsAndTempNames[varindex]);
    } else {
        TR::IlValue* args = builder->Load("returnSP");
        TR::IlValue* address = builder->IndexAt(stackElementPointerType, args, builder->ConstInt32(varindex));
        TR::IlValue* result = builder->LoadAt(stackElementPointerType, address);
        result = builder->ConvertTo(stackElementType, result);
        return result;
    }
}

void B9Method::storeVarIndex(TR::BytecodeBuilder* builder, int varindex, TR::IlValue* value)
{
    if (context->passParameters) {
        builder->Store(argsAndTempNames[varindex], value);
        return;
    } else {
        TR::IlValue* args = builder->Load("returnSP");
        TR::IlValue* address = builder->IndexAt(stackElementPointerType,
            args,
            builder->ConstInt32(varindex));
        builder->StoreAt(address, value);
    }
}

bool B9Method::generateILForBytecode(TR::BytecodeBuilder** bytecodeBuilderTable,
    uint8_t bytecode, long bytecodeIndex)
{
    TR::BytecodeBuilder* builder = bytecodeBuilderTable[bytecodeIndex];

    Instruction instruction = program[bytecodeIndex];

    assert(bytecode == getByteCodeFromInstruction(instruction));

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

    // printf("generating index=%d bc=%s(%d) param=%d \n", bytecodeIndex, b9_bytecodename(bytecode), bytecode, getParameterFromInstruction(instruction));

    if (context->debug) {
        QCOMMIT(builder);
        builder->Call("printVMState", 4, builder->Load("context"),
            builder->ConstInt64(bytecodeIndex),
            builder->ConstInt64(bytecode),
            builder->ConstInt64(getParameterFromInstruction(instruction)));
    }

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
    case RETURN: {
        auto result = pop(builder);
        if (!context->passParameters) {  
                builder->StoreIndirect("executionContextType", "stackPointer", builder->Load("context"), builder->Load("returnSP"));
        } 
        builder->Return(result);
    } break;
    case DROP:
        drop(builder);
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case JMP:
        handle_bc_jmp(builder, bytecodeBuilderTable, bytecodeIndex);
        break;
    case JMPLE:
        handle_bc_jmp_le(builder, bytecodeBuilderTable, bytecodeIndex, nextBytecodeBuilder);
        break;
    case SUB:
        handle_bc_sub(builder, nextBytecodeBuilder);
        break;
    case ADD:
        handle_bc_add(builder, nextBytecodeBuilder);
        break;
    case PUSH_CONSTANT: {
        int constvalue = getParameterFromInstruction(instruction);
        push(builder, builder->ConstInt64(constvalue));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case CALL: {
        int callindex = getParameterFromInstruction(instruction);
        Instruction* tocall = context->functions[callindex];
        if (context->directCall) { 
            uint64_t* slotForJitAddress = (uint64_t*)&tocall[1];
            if (tocall == program || *slotForJitAddress != 0) {
                const char* signature = "jit";
                int len = strlen(signature) + 16;
                char* nameToCall = (char*)malloc(len); // need to free later
                snprintf(nameToCall, len, "%s_%p", signature, tocall);
                if (tocall != program) {
                    int argsCount = progArgCount(*tocall);
                    DefineFunction((char*)nameToCall, (char*)__FILE__,
                        nameToCall, (void*)*slotForJitAddress, Int64, 2 + argsCount,
                        executionContextType, int32PointerType,
                        stackElementType, stackElementType, stackElementType, stackElementType,
                        stackElementType, stackElementType, stackElementType, stackElementType);
                }
                if (context->passParameters) {
                    int argsCount = progArgCount(*program);
                    if (argsCount > 8)
                        printf("ERROR Need to add handlers for more parameters\n");
                    TR::IlValue* p[8];
                    memset(p, 0, sizeof(p));
                    int popInto = argsCount;
                    while (popInto--) {
                        p[popInto] = pop(builder);
                    }
                    TR::IlValue* result = builder->Call(nameToCall, 2 + argsCount, builder->Load("context"), builder->ConstAddress(tocall),
                        p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
                    push(builder, result);
                } else {
                    QCOMMIT(builder);
                    TR::IlValue* result = builder->Call(nameToCall, 2, builder->Load("context"), builder->ConstAddress(tocall));
                    QRELOAD_DROP(builder, progArgCount(*tocall));
                    push(builder, result);
                }
            } else { // no address known in direct call, so dispatch intepreter
                QCOMMIT(builder);
                TR::IlValue* result = builder->Call("interpret", 2, builder->Load("context"), builder->ConstAddress(tocall));
                QRELOAD_DROP(builder, progArgCount(*tocall));
                push(builder, result);
            }
        } else {
            // only use interpreter to dispatch the calls
            QCOMMIT(builder);
            TR::IlValue* result = builder->Call("interpret", 2, builder->Load("context"), builder->ConstAddress(tocall));
            QRELOAD_DROP(builder, progArgCount(*tocall));
            push(builder, result);
        }

        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);

    } break;
    default:
        printf(" genIlForByteCode Failed, unrecognized byte code :%d\n", bytecode);
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
    StackElement delta = getParameterFromInstruction(instruction) + 1;
    int next_bc_index = bytecodeIndex + delta;
    TR::BytecodeBuilder* destBuilder = bytecodeBuilderTable[next_bc_index];
    builder->Goto(destBuilder);
}

void B9Method::handle_bc_jmp_le(TR::BytecodeBuilder* builder,
    TR::BytecodeBuilder** bytecodeBuilderTable,
    long bytecodeIndex,
    TR::BytecodeBuilder* nextBuilder)
{
    Instruction instruction = program[bytecodeIndex];

    StackElement delta = getParameterFromInstruction(instruction) + 1;

    TR::IlValue* right = pop(builder);
    TR::IlValue* left = pop(builder);

    int next_bc_index = bytecodeIndex + delta;
    TR::BytecodeBuilder* jumpTo = bytecodeBuilderTable[next_bc_index];

    left = builder->Sub(left, builder->ConstInt64(1));
    builder->IfCmpGreaterThan(jumpTo, right, left); //swap and do a greaterthan

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


TR::IlValue*
B9Method::pop(TR::BytecodeBuilder* builder)
{
    if (context->operandStack) {
        return QSTACK(builder)->Pop(builder);
    } else {
        TR::IlValue* sp = builder->LoadIndirect("executionContextType", "stackPointer", builder->Load("context"));
        TR::IlValue* newSP = builder->IndexAt(stackElementPointerType, sp, builder->ConstInt32(-1));
        builder->StoreIndirect("executionContextType", "stackPointer", builder->Load("context"), newSP);
        return builder->LoadAt(stackElementPointerType, newSP);
    }
}

void B9Method::push(TR::BytecodeBuilder* builder, TR::IlValue* value)
{
    if (context->operandStack) {
        return QSTACK(builder)->Push(builder, value);
    } else {
        TR::IlValue* sp = builder->LoadIndirect("executionContextType", "stackPointer", builder->Load("context"));
        builder->StoreAt(builder->ConvertTo(stackElementPointerType, sp), builder->ConvertTo(stackElementType, value));
        TR::IlValue* newSP = builder->IndexAt(stackElementPointerType, sp, builder->ConstInt32(1));
        builder->StoreIndirect("executionContextType", "stackPointer", builder->Load("context"), newSP);
    }
}
