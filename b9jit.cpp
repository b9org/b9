
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <dlfcn.h>
#include <errno.h>

#include "Jit.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"
// #include "j9.hpp"

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

void generateCodeForLambda(Instruction* program)
{
    TR::TypeDictionary types;
    B9Method methodBuilder(&types, program);
    uint8_t* entry = 0;
    int rc = (*compileMethodBuilder)(&methodBuilder, &entry);
    if (0 == rc) {
            printf("Compiled success address = <%p>\n", entry);
    } else {
            printf("Failed to compile");
    }
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
    DefineParameter("vm", p_b9_vm_struct); 
}

void B9Method::defineLocals()
{
    // DefineLocal("cachedVars", Int64);
    // DefineLocal("var0", Int64);
}

void B9Method::defineStructures(TR::TypeDictionary* types)
{
    pInt64 = types->PointerTo(Int64); 

    defineVMStructure(types);
    defineVMFrameStructure(types);
    defineVMObjectStructure(types);
}

void B9Method::defineFunctions()
{
    DefineFunction((char*)"printString", (char*)__FILE__, (char*)PRINTSTRING_LINE, (void*)&printString, NoType, 1, Int64);
    DefineFunction((char*)"printInt64", (char*)__FILE__, (char*)PRINTINT64_LINE, (void*)&printInt64, NoType, 1, Int64);
    DefineFunction((char*)"printInt64Hex", (char*)__FILE__, (char*)PRINTINT64HEX_LINE, (void*)&printInt64Hex, NoType, 1, Int64);

    DefineFunction((char*)"newline", (char*)__FILE__, (char*)NEWLINE_LINE, (void*)&newline, NoType, 0);
    DefineFunction((char*)"printstring", (char*)__FILE__, (char*)NEWLINE_LINE, (void*)&printstring, NoType, 1, Int64);


    void bc_call(ExecutionContext* context, uint16_t value);

    DefineFunction((char*)"bc_call", (char*)__FILE__, "bc_call", (void*)&bc_call, Int64, 2, Int64, Int64);
}
void B9Method::defineVMStructure(TR::TypeDictionary* types)
{
    b9_vm_struct = types->DefineStruct("b9_vm_struct");

//    types->DefineField("b9_vm_struct", "xxxxx", Int64, offsetof(struct _t4_vm_, known_constants.kc_null));
  
    types->CloseStruct("b9_vm_struct");

    p_b9_vm_struct = types->PointerTo("b9_vm_struct");
}

void B9Method::defineVMFrameStructure(TR::TypeDictionary* types)
{

    /*	struct _t4_frame_ {
    struct _t4_frame_* caller; // frame you return to
    t4_chunk method; // current method running in this frame
    t4_chunk vars; // with variables
    t4_chunk fthis; // this per frame, mainly null
    u8* spc; // only valid on prepared frames - ready for GC
    int args_passed; // only valid on prepared frames - ready for GC
};
*/

    types->DefineField("t4_frame_struct", "caller", Int64);
    types->DefineField("t4_frame_struct", "method", Int64);
    types->DefineField("t4_frame_struct", "vars", Int64);
    types->DefineField("t4_frame_struct", "fthis", Int64);
    types->DefineField("t4_frame_struct", "spc", Int64);
    types->DefineField("t4_frame_struct", "args_passed", Int64);

    types->CloseStruct("t4_frame_struct");
}

void B9Method::defineVMObjectStructure(TR::TypeDictionary* types)
{
    // t4_chunk_struct = types->DefineStruct("t4_chunk_struct");
    // types->DefineField("t4_chunk_struct", "omr_meta", Int64);
    // types->DefineField("t4_chunk_struct", "flags", Int64);
    // types->DefineField("t4_chunk_struct", "shape", Int64);
    // types->DefineField("t4_chunk_struct", "prototype", Int64);
    // types->DefineField("t4_chunk_struct", "acc", Int64);
    // types->DefineField("t4_chunk_struct", "first_data_offset", Int64);
    // types->CloseStruct("t4_chunk_struct");
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
    bytecodeBuilderTable[bytecodeIndex] = newBuilder;
}

bool B9Method::buildIL()
{
    TR::BytecodeBuilder** bytecodeBuilderTable = nullptr;
    bool success = true;

    OMR::VirtualMachineRegisterInStruct* stackTop
        = new OMR::VirtualMachineRegisterInStruct(this, "b9_vm_struct", "vm", "sp", "SP");
    OMR::VirtualMachineOperandStack* stack = new OMR::VirtualMachineOperandStack(this, 32, pInt64, stackTop);
    B9VirtualMachineState* vms = new B9VirtualMachineState(stack, stackTop);
    setVMState(vms);

#ifdef FIX_FIX_FIX
    //   printf("\nGenerating code for %s\n", t4_char_data_address(lambda->l.name.prop_value));
    t4_chunk code = lambda->l.function.prop_value;
    if (METHOD_CATCHES_THROW(code)) {
        //printf("\nSKIP due to exception handler\n" ) ;
        return false;
    }

    numberOfBytecodes = computeNumberOfBytecodes(code);

    long tableSize = sizeof(TR::BytecodeBuilder*) * numberOfBytecodes;
    bytecodeBuilderTable = (TR::BytecodeBuilder**)malloc(tableSize);
    if (NULL == bytecodeBuilderTable) {
        return false;
    }
    memset(bytecodeBuilderTable, 0, tableSize);

    bool* reachable = (bool*)malloc(tableSize);
    memset(reachable, 0, tableSize);

    int64_t i = METHOD_FIRST_BC_OFFSET;
    while (i < numberOfBytecodes) {
        reachable[i] = true;
        uint8_t bc = code->d.u8data[i];
        struct t4_bytecode_info* bc_info = t4_bcinfo_for_bc(bc);
        int nextbytecode = i + bc_info->length;
        switch (bc) {
        case T4_RETURN: {
            i = nextbytecode;
            while (!reachable[i] && i < numberOfBytecodes) {
                i++;
            }
        } break;

        case T4_JMP: {
            int delta = int16At(i + 1);
            int next_bc_index = nextbytecode + delta;
            reachable[next_bc_index] = true;
            i = nextbytecode;
            if (code->d.u8data[i] == T4_JMP_EXCEPTION) {
                reachable[i] = true;
            }
            while (!reachable[i] && i < numberOfBytecodes) {
                i++;
            }
        } break;
        case T4_JMP_EXCEPTION:
        case T4_JMP_NZ:
        case T4_JMP_EQ: {
            reachable[nextbytecode] = true;
            int delta = int16At(i + 1);
            int next_bc_index = nextbytecode + delta;
            reachable[next_bc_index] = true;
            i = nextbytecode;
        } break;
        default:
            // next bytecode fallthrough is reachable
            reachable[nextbytecode] = true;
            i = nextbytecode;
            break;
        }
    }

    i = METHOD_FIRST_BC_OFFSET;
    while (i < numberOfBytecodes) {
        uint8_t bc = code->d.u8data[i];
        struct t4_bytecode_info* bc_info = t4_bcinfo_for_bc(bc);
        if (reachable[i]) {
            createBuilderForBytecode(bytecodeBuilderTable, bc, i);
        } else {
            printBC("SKIP BC AT ", code, i, reachable[i]);
        }
        i += bc_info->length;
    }

    AppendBuilder(bytecodeBuilderTable[METHOD_FIRST_BC_OFFSET]);
    stackLevel = 0;
    i = METHOD_FIRST_BC_OFFSET;
    int prevBytecodeIndex = -1;
    while (i < numberOfBytecodes) {
        uint8_t bc = code->d.u8data[i];
        struct t4_bytecode_info* bc_info = t4_bcinfo_for_bc(bc);
        if (reachable[i]) {
            if (vm->trace.verbose) {
                printBC("generateILForBytecode", code, i, reachable[i]);
            }
            if (!generateILForBytecode(bytecodeBuilderTable, bc, i, prevBytecodeIndex)) {
                if (vm->trace.verbose) {
                    printBC("Failed to generateILForBytecode", code, i, reachable[i]);
                }
                success = false;
                break;
            }
            prevBytecodeIndex = i;
        }
        i += bc_info->length;
    }

    free((void*)bytecodeBuilderTable);
    return success;
#endif 
    return 0;
}

#ifdef FIX_FIX
TR::IlValue* B9Method::loadVarIndex(TR::BytecodeBuilder* builder, int varindex)
{
    TR::IlValue* vars = builder->LoadIndirect("t4_frame_struct", "vars", builder->Load("frame"));
    int offset = varindex + (offsetof(struct _t4_chunk_, d.pdata) / sizeof(Int64));
    TR::IlValue* address = builder->IndexAt(pInt64,
        vars,
        builder->ConstInt32(offset));
    return builder->LoadAt(pInt64, address);
}
void B9Method::storeVarIndex(TR::BytecodeBuilder* builder, int varindex, TR::IlValue* value)
{
    TR::IlValue* vars = builder->LoadIndirect("t4_frame_struct", "vars", builder->Load("frame"));
    int offset = varindex + (offsetof(struct _t4_chunk_, d.pdata) / sizeof(Int64));
    TR::IlValue* address = builder->IndexAt(pInt64,
        vars,
        builder->ConstInt32(offset));
    builder->StoreAt(address, value);
}

#endif  

bool B9Method::generateILForBytecode(TR::BytecodeBuilder** bytecodeBuilderTable,
    uint8_t bytecode, long bytecodeIndex, long prevBytecodeIndex)
{
    TR::BytecodeBuilder* builder = bytecodeBuilderTable[bytecodeIndex];

    // printf("generateILForBytecode builder %lx\n", (uint64_t)builder);

    if (NULL == builder) {
        printf("unexpected NULL BytecodeBuilder!\n");
        return false;
    }
#ifdef FIX_FIX
    struct t4_bytecode_info* bc_info = t4_bcinfo_for_bc(bytecode);
    TR::BytecodeBuilder* nextBytecodeBuilder = nullptr;
    long nextBytecodeIndex = bytecodeIndex + bc_info->length;
    if (nextBytecodeIndex < numberOfBytecodes) {
        nextBytecodeBuilder = bytecodeBuilderTable[nextBytecodeIndex];
    }

    if (vm->trace.debug) {
        QCOMMIT(builder);
        builder->Call("printFrame", 3,
            builder->Load("vm"),
            builder->Load("frame"),
            builder->ConstString(bc_info->name));
    }

    bool handled = true;
    switch (bytecode) {
    case T4_NOP:
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_THIS:
        push(builder, builder->LoadIndirect("t4_frame_struct", "fthis", builder->Load("frame")));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_MAGIC: {
        TR::IlValue* toPush = 0;
        switch (int8At(bytecodeIndex + 1)) {
        case 0:
            toPush = builder->LoadIndirect("b9_vm_struct", "t4_null", builder->Load("vm"));
            break;
        case 1:
            toPush = builder->LoadIndirect("b9_vm_struct", "t4_true", builder->Load("vm"));
            break;
        case 2:
            toPush = builder->LoadIndirect("b9_vm_struct", "t4_false", builder->Load("vm"));
            break;
        case 3:
            toPush = builder->LoadIndirect("b9_vm_struct", "t4_undefined", builder->Load("vm"));
            break;
        case 4:
            toPush = builder->LoadIndirect("b9_vm_struct", "t4_zero", builder->Load("vm"));
            break;
        case 5:
            toPush = builder->LoadIndirect("b9_vm_struct", "t4_neg_zero", builder->Load("vm"));
            break;
        case 6:
            toPush = builder->LoadIndirect("b9_vm_struct", "t4_NaN", builder->Load("vm"));
            break;
        default:
            break;
        }
        if (toPush) {
            push(builder, toPush);
            if (nextBytecodeBuilder)
                builder->AddFallThroughBuilder(nextBytecodeBuilder);
        } else {
            handled = false;
        }
    } break;

    case T4_PUSH_VAR:
        push(builder, loadVarIndex(builder, int8At(bytecodeIndex + 1)));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_POP_VAR:
        storeVarIndex(builder, int8At(bytecodeIndex + 1), pop(builder));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_VAR0:
        push(builder, loadVarIndex(builder, 0));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;

    case T4_POP_VAR0:
        storeVarIndex(builder, 0, pop(builder));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;

    case T4_PUSH_VAR1:
        push(builder, loadVarIndex(builder, 1));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_POP_VAR1:
        storeVarIndex(builder, 1, pop(builder));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_VAR2:
        push(builder, loadVarIndex(builder, 2));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_POP_VAR2:
        storeVarIndex(builder, 2, pop(builder));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_VAR3:
        push(builder, loadVarIndex(builder, 3));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_POP_VAR3:
        storeVarIndex(builder, 3, pop(builder));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_1:
        push(builder, builder->ConstInt64(T4_TAG_OBJECT(1)));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_2:
        push(builder, builder->ConstInt64(T4_TAG_OBJECT(2)));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_PUSH_3:
        push(builder, builder->ConstInt64(T4_TAG_OBJECT(3)));
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_RETURN: // BC_FORMAT_B
        QCOMMIT(builder);
        builder->Return(builder->ConstInt64(0));
        break;
    case T4_DROP: // BC_FORMAT_B
        drop(builder);
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
        break;
    case T4_COMPARE: {
        int comp = int8At(bytecodeIndex + 1);
        int nextbc = int8At(bytecodeIndex + 2);
        bool skip = (nextbc == T4_JMP_NZ) || (nextbc == T4_JMP_EQ);
        if (!skip) {
            handle_as_c_call_bb(builder, bc_info->jit_bc_name, comp);
        }
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;

    case T4_JMP: //BC_FORMAT_BW
        handle_bc_jmp(builder, bytecodeBuilderTable, bytecodeIndex);
        break;
    case T4_JMP_NZ: //BC_FORMAT_BW
        handle_bc_jmp_nz(builder, bytecodeBuilderTable, bytecodeIndex, prevBytecodeIndex, nextBytecodeBuilder);
        break;
    case T4_JMP_EQ: //BC_FORMAT_BW
        handle_bc_jmp_eq(builder, bytecodeBuilderTable, bytecodeIndex, prevBytecodeIndex, nextBytecodeBuilder);
        break;
    case T4_DEC:
        handle_bc_dec(builder, nextBytecodeBuilder);
        break;
    case T4_SUB:
        handle_bc_sub(builder, nextBytecodeBuilder);
        break;
    case T4_ADD:
        handle_bc_add(builder, nextBytecodeBuilder);
        break;
    case T4_DUP:
        handle_bc_dup(builder, nextBytecodeBuilder);
        break;

    case T4_PUSH_CONST: {
        int constValue = int16At(bytecodeIndex + 1);
        // printf("T4_PUSH_CONST %d\n", constValue);
        push(builder, builder->ConstInt64(T4_TAG_OBJECT(constValue)));
        // printf("DONE T4_PUSH_CONST %d\n", constValue);

        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;

    case T4_PUSH_LAMBDA: {
        int index = int16At(bytecodeIndex + 1);

        TR::IlValue* result = builder->Call("jit_push_lambda_ret",
            3,
            builder->Load("vm"),
            builder->Load("frame"),
            builder->ConstInt64(index));
        if (bc_info->format & BC_FORMAT_CHECKS_EXCEPTION) {
            handle_exception(builder);
        }
        push(builder, result);
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);
    } break;
    case T4_CALL: {
        int selectorIndex = int16At(bytecodeIndex + 1);
        int numberOfArgs = int8At(bytecodeIndex + 3);

        QCOMMIT(builder);
        TR::IlValue* result = builder->Call("jit_call_return_result", 4, builder->Load("vm"), builder->Load("frame"),
            builder->ConstInt64(selectorIndex), builder->ConstInt64(numberOfArgs));
        //QRELOAD(builder);
        if (numberOfArgs > 0) {
            QSTACK(builder)
                ->Drop(builder, numberOfArgs); // hack all args dropped
        }
        push(builder, result);
        if (bc_info->format & BC_FORMAT_CHECKS_EXCEPTION) {
            handle_exception(builder);
        }
        if (nextBytecodeBuilder)
            builder->AddFallThroughBuilder(nextBytecodeBuilder);

    } break;
    default:
        handled = false;
        break;
    }
    if (handled) {
        if (bc_info->jit_bc_name != 0) {
            printf("Can remove C handler for <%s> \n", bc_info->name);
        }
        return handled;
    }

    return handled;
#endif 
    return 0;
}

/*************************************************
 * GENERATE CODE FOR BYTECODES
 *************************************************/
 

void B9Method::handle_bc_jmp(TR::BytecodeBuilder* builder, TR::BytecodeBuilder** bytecodeBuilderTable, long bytecodeIndex)
{
#ifdef FIX_FIX
    int bc = int8At(bytecodeIndex);
    assert(bc == T4_JMP); // for now just be sure
    struct t4_bytecode_info* bc_info = t4_bcinfo_for_bc(bc);

    int delta = int16At(bytecodeIndex + 1); // jump <delta>
    int next_bc_index = bytecodeIndex + delta + bc_info->length;

    // printf("handle_bc_jmp delta is <%d>\n", delta);
    // printf("JUMPING TO pc @ %d\n", next_bc_index);
    // printBC("BYTECODE at Destination", code, next_bc_index);

    TR::BytecodeBuilder* destBuilder = bytecodeBuilderTable[next_bc_index];
    builder->Goto(destBuilder);
#endif
}
    

static int genBCIndex = 1024;
void B9Method::handle_bc_jmp_le(TR::BytecodeBuilder* builder,
    TR::BytecodeBuilder** bytecodeBuilderTable,
    long bytecodeIndex,
    int prevBytecodeIndex,
    TR::BytecodeBuilder* nextBuilder)
{
#ifdef FIX_FIX
    int bc = int8At(bytecodeIndex);
    assert(bc == T4_JMP_NZ); // for now just be sure
    struct t4_bytecode_info* bc_info = t4_bcinfo_for_bc(bc);

    int delta = int16At(bytecodeIndex + 1); // jmpnz <delta>
    int next_bc_index = bytecodeIndex + delta + bc_info->length;

    // printf("handle_bc_jmp_nz delta is <%d>\n", delta);
    // printf("JUMPING TO pc @ %d\n", next_bc_index);
    // printBC("BYTECODE at Destination", code, next_bc_index);

    TR::BytecodeBuilder* destBuilder = bytecodeBuilderTable[next_bc_index];
    // if (destBuilder)
    //     builder->AddSuccessorBuilder(&destBuilder);

    int prevBytecode = int8At(prevBytecodeIndex);
    if (prevBytecode == T4_COMPARE) {
        TR::IlValue* right = pop(builder);
        TR::IlValue* left = pop(builder);
        int comparetype = int8At(prevBytecodeIndex + 1);
        char* compareTypeFunction = compareType_to_name(comparetype);

        if (comparetype == COMPARE_LT) { // optimize compare LT inline
            TR::BytecodeBuilder* isTaggedPath = OrphanBytecodeBuilder(genBCIndex++, (char*)"handle_bc_jmp_nz_then");
            TR::BytecodeBuilder* isSlowPath = OrphanBytecodeBuilder(genBCIndex++, (char*)"handle_bc_jmp_nz_else");

            TR::IlValue* bothTagged = builder->And(builder->And(left, right), builder->ConstInt64(0x1));
            builder->IfCmpEqual(isSlowPath,
                builder->ConstInteger(Int64, 0),
                bothTagged);
            builder->AddFallThroughBuilder(isTaggedPath);

            //both are tagged path if (<= gets converted to jump if GreaterThan)
            isTaggedPath->IfCmpLessThan(destBuilder, right, left);
            isTaggedPath->IfCmpEqual(destBuilder, right, left);
            isTaggedPath->AddFallThroughBuilder(nextBuilder);

            // slow path
            TR::IlValue* true_or_false = isSlowPath->Call(compareTypeFunction, 4,
                isSlowPath->Load("vm"),
                isSlowPath->Load("frame"), left, right);
            isSlowPath->IfCmpEqual(destBuilder, true_or_false, isSlowPath->ConstInt64(0));
            isSlowPath->Goto(nextBuilder);
        } else {
            // handle the previously skipped compare, together with the jump
            commitPrint(vm, builder, "CALLING slow function for compare ");
            TR::IlValue* true_or_false = builder->Call(compareTypeFunction, 4, builder->Load("vm"),
                builder->Load("frame"), left, right);
            builder->IfCmpEqual(destBuilder, true_or_false, builder->ConstInt64(0)); // converted isFalse to 0/1
            builder->AddFallThroughBuilder(nextBuilder);
        }
        return;
    }
    int true_false_guaranteed = (prevBytecode == T4_NOT);
    if (true_false_guaranteed) {
        TR::IlValue* value = pop(builder);
        TR::IlValue* t4_false = builder->LoadIndirect("b9_vm_struct", "t4_false", builder->Load("vm"));
        builder->IfCmpEqual(destBuilder, value, t4_false); // converted isFalse to 0/1

    } else {
        TR::IlValue* value = pop(builder);
        TR::IlValue* isFalseHood = isFalsehood(builder, value);
        builder->IfCmpEqual(destBuilder, isFalseHood,
            builder->ConstInt64((int64_t)1)); // converted isFalse to 0/1
    }
    builder->AddFallThroughBuilder(nextBuilder);
#endif
}

void B9Method::handle_bc_sub(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* nextBuilder)
{
#ifdef FIX_FIX
    TR::BytecodeBuilder* isTaggedPath = OrphanBytecodeBuilder(genBCIndex++, (char*)"handle_bc_sub_then");
    TR::BytecodeBuilder* isSlowPath = OrphanBytecodeBuilder(genBCIndex++, (char*)"handle_bc_sub_else");

    TR::IlValue* right = pop(builder);
    TR::IlValue* left = pop(builder);

    TR::IlValue* bothTagged = builder->And(builder->And(left, right), builder->ConstInt64(0x1));
    builder->IfCmpEqual(isSlowPath,
        builder->ConstInteger(Int64, 0),
        bothTagged);
    builder->AddFallThroughBuilder(isTaggedPath);

    // tagged path
    push(isTaggedPath, isTaggedPath->Sub(left, isTaggedPath->Sub(right, isTaggedPath->ConstInt64(1))));
    isTaggedPath->AddFallThroughBuilder(nextBuilder);

    // slow path
    TR::IlValue* result = isSlowPath->Call("jit_sub_lr", 4, isSlowPath->Load("vm"), isSlowPath->Load("frame"), left, right);
    push(isSlowPath, result);

    isSlowPath->Goto(nextBuilder);
#endif

}

void B9Method::handle_bc_add(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* nextBuilder)
{
#ifdef FIX_FIX
    TR::BytecodeBuilder* isTaggedPath = OrphanBytecodeBuilder(genBCIndex++, (char*)"handle_bc_add_then");
    TR::BytecodeBuilder* isSlowPath = OrphanBytecodeBuilder(genBCIndex++, (char*)"handle_bc_add_else");

    TR::IlValue* right = pop(builder);
    TR::IlValue* left = pop(builder);
    TR::IlValue* bothTagged = builder->And(builder->And(left, right), builder->ConstInt64(0x1));
    builder->IfCmpEqual(isSlowPath,
        builder->ConstInteger(Int64, 0),
        bothTagged);
    builder->AddFallThroughBuilder(isTaggedPath);

    // tagged path
    push(isTaggedPath, isTaggedPath->Add(left, isTaggedPath->Sub(right, isTaggedPath->ConstInt64(1))));
    isTaggedPath->AddFallThroughBuilder(nextBuilder);

    // slow path
    TR::IlValue* result = isSlowPath->Call("jit_add_lr", 4, isSlowPath->Load("vm"), isSlowPath->Load("frame"), left, right);
    push(isSlowPath, result);

    isSlowPath->Goto(nextBuilder);
#endif
}

void B9Method::drop(TR::BytecodeBuilder* builder)
{ 
    if (hackVMState) {
        // printf("enableVMState drop <%s> stack %d\n", __func__, stackLevel);
        QSTACK(builder)
            ->Drop(builder, 1);
    } else {
        TR::IlValue* sp = builder->LoadIndirect("b9_vm_struct", "sp", builder->Load("vm"));
        TR::IlValue* newSP = builder->Sub(sp, builder->ConstInt64(8));
        builder->StoreIndirect("b9_vm_struct", "sp", builder->Load("vm"), newSP);
    }
}

TR::IlValue*
B9Method::peek(TR::BytecodeBuilder* builder)
{
    if (hackVMState) { 
        return QSTACK(builder)->Top();
    } else {
        TR::IlValue* sp = builder->LoadIndirect("b9_vm_struct", "sp", builder->Load("vm"));
        TR::IlValue* tos = builder->Sub(sp, builder->ConstInt64(8));
        TR::IlValue* value = builder->LoadAt(pInt64, tos);
        return value;
    }
}

TR::IlValue* B9Method::pop(TR::BytecodeBuilder* builder)
{ 

    if (hackVMState) {
        //printf("QPOP <%s> stack %d\n", __func__, stackLevel);
        return QSTACK(builder)->Pop(builder);
    } else {
        // return *--sp
        TR::IlValue* sp = builder->LoadIndirect("b9_vm_struct", "sp", builder->Load("vm"));
        TR::IlValue* newSP = builder->Sub(sp, builder->ConstInt64(8));
        builder->StoreIndirect("b9_vm_struct", "sp", builder->Load("vm"), newSP);
        TR::IlValue* value = builder->LoadAt(pInt64, newSP);
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
        TR::IlValue* sp = builder->LoadIndirect("b9_vm_struct", "sp", builder->Load("vm"));
        builder->StoreAt(sp, value);
        TR::IlValue* newSP = builder->Add(sp, builder->ConstInt64(8));
        builder->StoreIndirect("b9_vm_struct", "sp", builder->Load("vm"), newSP);
    }
} 
