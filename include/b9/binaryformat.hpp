#if !defined(B9_BINARY_HPP_)
#define B9_BINARYFORMAT_HPP_

#include <b9/core.hpp>

namespace b9 {

struct PrimitiveData {
    const char * name;
    PrimitiveFunction *address;
};

class ExportedFunctionData {
public:
    int functionCount_;
    FunctionSpecification *functionTable_;
};

} // namespace b9

#endif // B9_BINARYFORMAT_HPP_
