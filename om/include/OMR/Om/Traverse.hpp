#if !defined(OMR_OM_TRAVERSE_HPP_)
#define OMR_OM_TRAVERSE_HPP_

#include <OMR/Om/Context.hpp>
#include <OMR/Om/Cell.hpp>

namespace OMR {
namespace Om {

class Visitor {
 public:
  /// An edge from Root A to Cell B.
  virtual void rootEdge(Context& cx, void* a, Cell* b) = 0;

  /// An edge from Cell A to Cell B.
  virtual void edge(Context& cx, Cell* a, Cell* b) = 0;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_TRAVERSE_HPP_
