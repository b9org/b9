#if !defined(OMR_OM_MARKINGFN_HPP_)
#define OMR_OM_MARKINGFN_HPP_

#include <functional>

namespace OMR {
namespace Om {

class Context;
class Visitor;

using MarkingFn = std::function<void(Context&, Visitor&)>;

using MarkingFnVector = std::vector<MarkingFn>;

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MARKINGFN_HPP_
