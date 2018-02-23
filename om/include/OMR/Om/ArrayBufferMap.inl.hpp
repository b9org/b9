#if !defined(OMR_OM_ARRAYBUFFERMAP_INL_HPP_)
#define OMR_OM_ARRAYBUFFERMAP_INL_HPP_

#include <OMR/Om/Allocator.hpp>
#include <OMR/Om/Map.inl.hpp>

namespace OMR {
namespace Om {

inline ArrayBufferMap::ArrayBufferMap(MetaMap* meta)
    : base_{{meta, Map::Kind::ARRAY_BUFFER_MAP}} {}

struct ArrayBufferMapInitializer : public Initializer {
  Cell* operator()(Context& cx, Cell* cell) {
    auto meta = cx.globals().metaMap();
    new (cell) ArrayBufferMap(meta);
    return cell;
  }
};

inline ArrayBufferMap* ArrayBufferMap::allocate(Context& cx) {
  ArrayBufferMapInitializer init;
  return BaseAllocator::allocate<ArrayBufferMap>(cx, init);
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_ARRAYBUFFERMAP_INL_HPP_
