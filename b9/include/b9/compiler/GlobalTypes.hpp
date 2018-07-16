#if !defined(B9_GLOBALTYPES_HPP_)
#define B9_GLOBALTYPES_HPP_

#include <ilgen/TypeDictionary.hpp>

namespace b9 {

/// A collection of basic, built in types.
class GlobalTypes {
 public:
  GlobalTypes(TR::TypeDictionary &td);

  TR::IlType *size;
  TR::IlType *addressPtr;
  TR::IlType *int64Ptr;
  TR::IlType *int32Ptr;
  TR::IlType *int16Ptr;

  TR::IlType *stackElement;
  TR::IlType *stackElementPtr;
  TR::IlType *instruction;
  TR::IlType *instructionPtr;

  TR::IlType *operandStack;
  TR::IlType *operandStackPtr;
  TR::IlType *executionContext;
  TR::IlType *executionContextPtr;
};

}  // namespace b9

#endif  // B9_GLOBALTYPES_HPP_