#if !defined(B9_TRAVERSE_HPP_)
#define B9_TRAVERSE_HPP_

#include <b9/context.hpp>
#include <b9/objects.hpp>

namespace b9 {

class Visitor {
 public:
  /// An edge from Root A to Cell B.
  virtual void rootEdge(Context& cx, void* a, Cell* b) = 0;

  /// An edge from Cell A to Cell B.
  virtual void edge(Context& cx, Cell* a, Cell* b) = 0;
};

}  // namespace b9

#endif  // B9_TRAVERSE_HPP_
