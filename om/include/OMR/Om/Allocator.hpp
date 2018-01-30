#ifndef OMR_OM_ALLOCATOR_HPP_
#define OMR_OM_ALLOCATOR_HPP_

#include <OMR/Om/Allocation.hpp>
#include <cstdlib>

namespace OMR {
namespace Om {

struct Cell;

constexpr bool DEBUG_SLOW = true;

struct BaseAllocator {
  template <typename ResultT = Cell, typename InitializerT>
  static ResultT* allocate(Context& cx, InitializerT& init,
                           std::size_t size = sizeof(ResultT));
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ALLOCATOR_HPP_
