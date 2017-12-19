#if !defined(OMR_OM_MARKING_HPP_)
#define OMR_OM_MARKING_HPP_

#include <OMR/Om/Traverse.hpp>

#include <MarkingScheme.hpp>

namespace OMR {
namespace Om {

class Marker : public Visitor {
 public:
  Marker(MM_MarkingScheme* omrMarker) : omrMarker_(omrMarker) {}

  virtual void edge(Context& cx, Cell* a, Cell* b) override {
    omrMarker_->markObject(cx.omrGcThread(), b);
  }

  virtual void rootEdge(Context& cx, void* a, Cell* b) override {
    if (b != nullptr) omrMarker_->markObject(cx.omrGcThread(), b, true);
  }

  MM_MarkingScheme* omrMarker_;
};

}  // namespace Om
}  // namespace OMR

#endif  // B9_MARKING_HPP_
