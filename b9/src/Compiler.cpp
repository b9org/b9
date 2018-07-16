#include "b9/compiler/Compiler.hpp"
#include "b9/ExecutionContext.hpp"
#include "b9/VirtualMachine.hpp"
#include "b9/compiler/GlobalTypes.hpp"
#include "b9/compiler/MethodBuilder.hpp"
#include "b9/instructions.hpp"

#include <dlfcn.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace b9 {

GlobalTypes::GlobalTypes(TR::TypeDictionary &td) {
  // Core Integer Types

  size = td.toIlType<std::size_t>();
  addressPtr = td.PointerTo(TR::Address);
  int64Ptr = td.PointerTo(TR::Int64);
  int32Ptr = td.PointerTo(TR::Int32);
  int16Ptr = td.PointerTo(TR::Int16);

  // Basic VM Data

  stackElement = td.toIlType<Om::RawValue>();
  stackElementPtr = td.PointerTo(stackElement);

  instruction = td.toIlType<RawInstruction>();
  instructionPtr = td.PointerTo(instruction);

  // VM Structures

  auto os = "b9::OperandStack";
  operandStack = td.DefineStruct(os);
  td.DefineField(os, "top_", td.PointerTo(stackElementPtr),
                 OperandStackOffset::TOP);
  td.DefineField(os, "stack_", stackElementPtr, OperandStackOffset::STACK);
  td.CloseStruct(os);

  operandStackPtr = td.PointerTo(operandStack);

  auto ec = "b9::ExecutionContext";
  executionContext = td.DefineStruct(ec);
  // td.DefineField(ec, "omContext", ???, ExecutionContextOffset::OM_CONTEXT);
  td.DefineField(ec, "stack_", operandStack, ExecutionContextOffset::STACK);
  // td.DefineField(ec, "programCounter", ???,
  // ExecutionContextOffset::PROGRAM_COUNTER);
  td.CloseStruct(ec);

  executionContextPtr = td.PointerTo(executionContext);
}

Compiler::Compiler(VirtualMachine &virtualMachine, const Config &cfg)
    : typeDictionary_(),
      globalTypes_(typeDictionary_),
      virtualMachine_(virtualMachine),
      cfg_(cfg) {}

JitFunction Compiler::generateCode(const std::size_t functionIndex) {
  const FunctionDef *function = virtualMachine_.getFunction(functionIndex);
  MethodBuilder methodBuilder(virtualMachine_, functionIndex);

  if (cfg_.verbose)
    std::cout << "MethodBuilder for function: " << function->name
              << " is constructed" << std::endl;

  uint8_t *result = nullptr;
  auto rc = compileMethodBuilder(&methodBuilder, &result);

  if (rc != 0) {
    std::cout << "Failed to compile function: " << function->name
              << " nparams: " << function->nparams << std::endl;
    throw b9::CompilationException{"IL generation failed"};
  }

  if (cfg_.verbose)
    std::cout << "Compilation completed with return code: " << rc
              << ", code address: " << static_cast<void *>(result) << std::endl;

  return (JitFunction)result;
}

}  // namespace b9
