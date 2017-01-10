
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

extern const char * b9_bytecodename(int bc);
void printVMState (ExecutionContext *context, int64_t pc, ByteCode bytecode, Parameter param)
{
    printf ("Executing at pc %d, bc is (%d, %s), param is (%d)\n", pc, bytecode, b9_bytecodename(bytecode), param);
    b9PrintStack (context);
}

class B9VirtualMachineState : public OMR::VirtualMachineState
{
public:
    B9VirtualMachineState()
        : OMR::VirtualMachineState(), _stack(NULL), _stackTop(NULL)
    {
    }

    B9VirtualMachineState(OMR::VirtualMachineOperandStack *stack, OMR::VirtualMachineRegister *stackTop)
        : OMR::VirtualMachineState(), _stack(stack), _stackTop(stackTop)
    {
    }

    virtual void
    Commit(TR::IlBuilder *b)
    {
        _stack->Commit(b);
        _stackTop->Commit(b);
    }

    virtual void
    Reload(TR::IlBuilder *b)
    {
        _stackTop->Reload(b);
        _stack->Reload(b);
    }

    virtual VirtualMachineState *
    MakeCopy()
    {
        B9VirtualMachineState *newState = new B9VirtualMachineState();
        newState->_stack = (OMR::VirtualMachineOperandStack *)_stack->MakeCopy();
        newState->_stackTop = (OMR::VirtualMachineRegister *)_stackTop->MakeCopy();
        return newState;
    }

    virtual void
    MergeInto(VirtualMachineState *other, TR::IlBuilder *b)
    {
        B9VirtualMachineState *otherState = (B9VirtualMachineState *)other;
        _stack->MergeInto(otherState->_stack, b);
        _stackTop->MergeInto(otherState->_stackTop, b);
    }

    OMR::VirtualMachineOperandStack *_stack;
    OMR::VirtualMachineRegister *_stackTop;
};

void
b9_jit_init()
{
    initializeJit();
}

void
generateCode(Instruction *program)
{
    TR::TypeDictionary types;
    B9Method methodBuilder(&types, program);
    uint8_t *entry = 0;
    printf("Start gen code\n");
    int rc = (*compileMethodBuilder)(&methodBuilder, &entry);
    if (0 == rc) {
        printf("Compiled success address = <%p>\n", entry);
        uint64_t *slotForJitAddress = (uint64_t *)&program[1];
        *slotForJitAddress = (uint64_t)entry;
    } else {
        printf("Failed to compile");
    }
    printf("Done gen code\n");
}



B9Method::B9Method(TR::TypeDictionary *types, Instruction *program)
    : MethodBuilder(types), program(program)
{
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);

    const char *signature = "jit";
    int len = strlen(signature) + 16;
    char *methodName = (char *)malloc(len);  
    snprintf(methodName, len, "%s_%p", signature, program);
    printf("Generating from %p -> <%s>\n", program, methodName);

    DefineName(methodName);
    if (sizeof(stack_element_t) == 2)  { 
            StackElement = Int16; 
            pStackElement = types->PointerTo(StackElement);
    }
    if (sizeof(stack_element_t) == 4) {
            StackElement=Int32; 
            pStackElement = types->PointerTo(StackElement);
    } 
    if (sizeof(stack_element_t) == 8) {
            StackElement = Int64; 
            pStackElement = types->PointerTo(StackElement);
    }

    DefineReturnType(StackElement);

    defineStructures(types);
    defineParameters();
    defineLocals();
    defineFunctions();

    AllLocalsHaveBeenDefined();
}


#if PASS_PARAMETERS_DIRECTLY
static const char *argsAndTempNames[]= 
{"arg0", "arg1", "arg2", "arg3", "arg4", "arg5", "arg6", "arg7", "arg8", "arg9"};
#endif 

void
B9Method::defineParameters()
{
    DefineParameter("context", p_b9_execution_context);
    DefineParameter("program", pInt32);
#if PASS_PARAMETERS_DIRECTLY
    int argsCount = progArgCount(*program);
    for (int i = 0;i<argsCount;i++) { 
        printf ("Arg %d defined <%s>\n", i, argsAndTempNames[i]);
        DefineParameter(argsAndTempNames[i], Int64); 
    }
#endif
}

void
B9Method::defineLocals()
{
    int argsCount = progArgCount(*program);
    int tempCount = progTmpCount(*program); 
#if PASS_PARAMETERS_DIRECTLY
    for (int i = argsCount;i<argsCount+tempCount;i++) { 
        printf ("Temp %d defined <%s>\n", i, argsAndTempNames[i]);
        DefineLocal(argsAndTempNames[i], Int64); 
    }
#else
    DefineLocal("returnSP", Int64);
#endif
}

void
B9Method::defineStructures(TR::TypeDictionary *types)
{
    pInt64 = types->PointerTo(Int64);
    pInt32 = types->PointerTo(Int32);
    pInt16 = types->PointerTo(Int16);

    b9_execution_context = types->DefineStruct("b9_execution_context");
    types->DefineField("b9_execution_context", "stack", pStackElement, offsetof(struct ExecutionContext, stack));
    types->DefineField("b9_execution_context", "stackPointer", pStackElement, offsetof(struct ExecutionContext, stackPointer));
    types->DefineField("b9_execution_context", "functions", pInt64, offsetof(struct ExecutionContext, functions));
    types->CloseStruct("b9_execution_context");

    p_b9_execution_context = types->PointerTo(b9_execution_context);
}

long
getargs(Instruction p)
{
    return progArgCount(p);
}

long
gettemps(Instruction p)
{
    return progTmpCount(p);
}

void
B9Method::defineFunctions()
{
    DefineFunction((char *)"getargs", (char *)__FILE__, "getargs", (void *)&getargs, Int64, 1, Int64);
    DefineFunction((char *)"gettemps", (char *)__FILE__, "gettemps", (void *)&gettemps, Int64, 1, Int64);

    DefineFunction((char *)"printVMState", (char *)__FILE__, "printVMState", (void *)&printVMState, NoType, 4, Int64, Int64, Int64, Int64);
    DefineFunction((char *)"printStack", (char *)__FILE__, "printStack", (void *)&b9PrintStack, NoType, 1, Int64);

    DefineFunction((char *)"interpret", (char *)__FILE__, "interpret", (void *)&interpret, Int64, 2, Int64, Int64);
}

#if USE_VM_OPERAND_STACK
#define QSTACK(b) (((B9VirtualMachineState *)(b)->vmState())->_stack)
#define QCOMMIT(b) ((b)->vmState()->Commit(b));  
#define QRELOAD(b)  ((b)->vmState()->Reload(b)); 
#else 
#define QCOMMIT(b)   
#define QRELOAD(b)    
#endif 

const char *
b9_bytecodename(int bc)
{

    if (bc==PUSH_CONSTANT) return "PUSH_CONSTANT" ;
    if (bc==DROP) return "DROP" ;
    if (bc==PUSH_FROM_VAR) return "PUSH_FROM_VAR" ;
    if (bc==POP_INTO_VAR) return "POP_INTO_VAR" ;
    if (bc==SUB) return "SUB" ;
    if (bc==ADD) return "ADD" ;
    if (bc==CALL) return "CALL" ;
    if (bc==RETURN) return "RETURN" ;
    if (bc==JMPLE) return "JMPLE" ;
    if (bc==JMP) return "JMP" ;
    return "unknown bc";
}



void
B9Method::createBuilderForBytecode(TR::BytecodeBuilder **bytecodeBuilderTable, uint8_t bytecode, int64_t bytecodeIndex)
{
    TR::BytecodeBuilder *newBuilder = OrphanBytecodeBuilder(bytecodeIndex, (char *)b9_bytecodename(bytecode));
    //printf("Created bytecodebuilder index=%d bc=%d param=%d %p\n", bytecodeIndex, bytecode, getParameterFromInstruction(program[bytecodeIndex]), newBuilder);
    bytecodeBuilderTable[bytecodeIndex] = newBuilder;
}

long
computeNumberOfBytecodes(Instruction *program)
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

bool
B9Method::buildIL()
{
    TR::BytecodeBuilder **bytecodeBuilderTable = nullptr;
    bool success = true;

    OMR::VirtualMachineRegisterInStruct *stackTop = new OMR::VirtualMachineRegisterInStruct(this, "b9_execution_context", "context", "stackPointer", "SP");
    OMR::VirtualMachineOperandStack *stack = new OMR::VirtualMachineOperandStack(this, 32, pStackElement, stackTop);
    B9VirtualMachineState *vms = new B9VirtualMachineState(stack, stackTop);
    setVMState(vms);

    long numberOfBytecodes = computeNumberOfBytecodes(program);

    long tableSize = sizeof(TR::BytecodeBuilder *) * numberOfBytecodes;
    bytecodeBuilderTable = (TR::BytecodeBuilder **)malloc(tableSize);
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
    TR::BytecodeBuilder *builder = bytecodeBuilderTable[METHOD_FIRST_BC_OFFSET];
  //printf("builder %p\n", builder);
    AppendBuilder(builder);

#if !PASS_PARAMETERS_DIRECTLY 
    TR::IlValue *nargs = builder->ConstInt32(progArgCount(*program) * sizeof(stack_element_t));
    TR::IlValue *tmps = builder->ConstInt32(progTmpCount(*program) * sizeof(stack_element_t));

    TR::IlValue *sp = builder->LoadIndirect("b9_execution_context", "stackPointer", builder->Load("context"));
    TR::IlValue *args = builder->Sub(sp, nargs);
    builder->Store("returnSP", args);
    sp = builder->Add(sp, tmps);
    builder->StoreIndirect("b9_execution_context", "stackPointer", builder->Load("context"), sp);
#endif


    i = METHOD_FIRST_BC_OFFSET;
    while (i < numberOfBytecodes) {
        Instruction bc = getByteCodeFromInstruction(program[i]);
        if (!generateILForBytecode(bytecodeBuilderTable, bc, i)) {
            success = false;
            break;
        }

        i += 1;
    }

    free((void *)bytecodeBuilderTable);
    return success;
}

TR::IlValue *
B9Method::loadVarIndex(TR::BytecodeBuilder *builder, int varindex)
{
#if PASS_PARAMETERS_DIRECTLY
    return builder->Load(argsAndTempNames[varindex]);
#else 
    TR::IlValue *args = builder->Load("returnSP");
    TR::IlValue *address = builder->IndexAt(pStackElement, args, builder->ConstInt32(varindex)); 
    TR::IlValue *result = builder->LoadAt(pStackElement, address);  
    result = builder->ConvertTo(Int64, result); 
    return result;
#endif
}

void
B9Method::storeVarIndex(TR::BytecodeBuilder *builder, int varindex, TR::IlValue *value)
{
#if PASS_PARAMETERS_DIRECTLY
    builder->Store(argsAndTempNames[varindex], value);
    return; 
#else 
    TR::IlValue *args = builder->Load("returnSP");
    TR::IlValue *address = builder->IndexAt(pStackElement,
        args,
        builder->ConstInt32(varindex));
    builder->StoreAt(address, value);
#endif 

}

bool
B9Method::generateILForBytecode(TR::BytecodeBuilder **bytecodeBuilderTable,
    uint8_t bytecode, long bytecodeIndex)
{
    TR::BytecodeBuilder *builder = bytecodeBuilderTable[bytecodeIndex];

    
    Instruction instruction = program[bytecodeIndex];

    assert(bytecode == getByteCodeFromInstruction(instruction));

    if (NULL == builder) {
        printf("unexpected NULL BytecodeBuilder!\n");
        return false;
    }

    long numberOfBytecodes = computeNumberOfBytecodes(program);
    TR::BytecodeBuilder *nextBytecodeBuilder = nullptr;
    long nextBytecodeIndex = bytecodeIndex + 1;
    if (nextBytecodeIndex < numberOfBytecodes) {
        nextBytecodeBuilder = bytecodeBuilderTable[nextBytecodeIndex];
    }

    bool handled = true;

    printf("generating index=%d bc=%s(%d) param=%d \n", bytecodeIndex, b9_bytecodename(bytecode), bytecode, getParameterFromInstruction(instruction));

    // if (bytecodeIndex == METHOD_FIRST_BC_OFFSET) {
        // QCOMMIT (builder);
        // builder->Call("printVMState", 4, builder->Load("context"), 
        //     builder->ConstInt64(bytecodeIndex),
        //     builder->ConstInt64(bytecode),
        //     builder->ConstInt64(getParameterFromInstruction(instruction))
        //     );
    // }

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
#if !PASS_PARAMETERS_DIRECTLY
        builder->StoreIndirect("b9_execution_context", "stackPointer", builder->Load("context"), builder->Load("returnSP"));
#endif
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
#if USE_DIRECT_CALL
        extern  Instruction *functions[]; 
        Instruction *tocall = functions[callindex]; 
        char *nameToCall = 0; 
        uint64_t *slotForJitAddress = (uint64_t *)&tocall[1];
        if (tocall == program || *slotForJitAddress != 0) { 
            const char *signature = "jit";
            int len = strlen(signature) + 16;
            nameToCall = (char *)malloc(len);  // need to free later 
            snprintf(nameToCall, len, "%s_%p", signature, tocall); 
            if (tocall != program ) {
              DefineFunction((char *)nameToCall, (char *)__FILE__, nameToCall, (void *)*slotForJitAddress , Int64, 2, Int64, Int64);
            }         
        } else {  
                nameToCall = "interpret";
        } 

#if PASS_PARAMETERS_DIRECTLY
        int argsCount = progArgCount(*program); 
        TR::IlValue *result = 0;
        switch (argsCount) {
            case 0:  
            result = 
                builder->Call(nameToCall, 2, builder->Load("context"), builder->ConstAddress(tocall));
            break;
           case  1:   {   
                TR::IlValue * p1 = pop(builder);
                result = builder->Call(nameToCall, 3, builder->Load("context"), builder->ConstAddress(tocall), p1); 
           }
           break; 
            case 2:  {    
                TR::IlValue * p2 = pop(builder);
                TR::IlValue * p1 = pop(builder);
                result = builder->Call(nameToCall, 4, builder->Load("context"), builder->ConstAddress(tocall),p1,p2); 
           }
            break;
            case 3: {
                TR::IlValue * p3 = pop(builder);
                TR::IlValue * p2 = pop(builder);
                TR::IlValue * p1 = pop(builder);
                result = builder->Call(nameToCall, 5, builder->Load("context"), builder->ConstAddress(tocall),p1,p2,p3);
            }
            break;
            default: 
                printf ("Need to add handlers for more parameters\n");
                break;
        } 
#else 
    QCOMMIT(builder);
    TR::IlValue *result = 
                builder->Call(nameToCall, 2, builder->Load("context"), builder->ConstAddress(tocall));      
#endif 

#if !PASS_PARAMETERS_DIRECTLY
#if USE_VM_OPERAND_STACK  //locally inform the operand stck the parms are consumed by the call
            QSTACK(builder)->Drop(builder, progArgCount(*tocall));
#endif 
#endif 

#else
        TR::IlValue *fTable = builder->LoadIndirect("b9_execution_context", "functions", builder->Load("context"));
        TR::IlValue *address = builder->IndexAt(pInt64, fTable, builder->ConstInt32(callindex));
        QCOMMIT(builder);
        TR::IlValue *result = builder->Call("interpret", 2, builder->Load("context"), builder->LoadAt(pInt64, address));
#endif 
       push(builder, result);

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

void
B9Method::handle_bc_jmp(TR::BytecodeBuilder *builder, TR::BytecodeBuilder **bytecodeBuilderTable, long bytecodeIndex)
{
    Instruction instruction = program[bytecodeIndex]; 
    stack_element_t  delta = getParameterFromInstruction(instruction) + 1; 
    int next_bc_index = bytecodeIndex + delta; 
    TR::BytecodeBuilder *destBuilder = bytecodeBuilderTable[next_bc_index];
    builder->Goto(destBuilder);
}

void
B9Method::handle_bc_jmp_le(TR::BytecodeBuilder *builder,
    TR::BytecodeBuilder **bytecodeBuilderTable,
    long bytecodeIndex,
    TR::BytecodeBuilder *nextBuilder)
{
    Instruction instruction = program[bytecodeIndex];

    stack_element_t delta = getParameterFromInstruction(instruction) + 1;

    TR::IlValue *right = pop(builder);
    TR::IlValue *left = pop(builder);

    int next_bc_index = bytecodeIndex + delta;
    TR::BytecodeBuilder *jumpTo = bytecodeBuilderTable[next_bc_index];

    builder->IfCmpLessThan(jumpTo, left, right);
    builder->IfCmpEqual(jumpTo, left, right);

    builder->AddFallThroughBuilder(nextBuilder);
}

void
B9Method::handle_bc_sub(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder)
{
    TR::IlValue *right = pop(builder);
    TR::IlValue *left = pop(builder);

    push(builder, builder->Sub(left, right));
    builder->AddFallThroughBuilder(nextBuilder);
}

void
B9Method::handle_bc_add(TR::BytecodeBuilder *builder, TR::BytecodeBuilder *nextBuilder)
{
    TR::IlValue *right = pop(builder);
    TR::IlValue *left = pop(builder);

    push(builder, builder->Add(left, right));
    builder->AddFallThroughBuilder(nextBuilder);
}

void
B9Method::drop(TR::BytecodeBuilder *builder)
{
    pop(builder);
}

TR::IlValue *
B9Method::pop(TR::BytecodeBuilder *builder)
{
#if USE_VM_OPERAND_STACK
        return QSTACK(builder)->Pop(builder);
#else
        TR::IlValue *sp = builder->LoadIndirect("b9_execution_context", "stackPointer", builder->Load("context"));
        TR::IlValue *newSP = builder->IndexAt(pStackElement, sp, builder->ConstInt32(-1));

        builder->StoreIndirect("b9_execution_context", "stackPointer", builder->Load("context"), newSP);

        return builder->LoadAt(pStackElement, newSP);;
#endif
}

void
B9Method::push(TR::BytecodeBuilder *builder, TR::IlValue *value)
{
#if USE_VM_OPERAND_STACK
        return QSTACK(builder)->Push(builder, value);
#else
        TR::IlValue *sp = builder->LoadIndirect("b9_execution_context", "stackPointer", builder->Load("context"));
        builder->StoreAt(builder->ConvertTo(pStackElement, sp), builder->ConvertTo(StackElement, value));

        TR::IlValue *newSP = builder->IndexAt(pStackElement, sp, builder->ConstInt32(1));

        builder->StoreIndirect("b9_execution_context", "stackPointer", builder->Load("context"), newSP);
#endif 
}
