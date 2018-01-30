#if !defined(OMR_OM_METAMAPALLOCATOR_HPP_)
#define OMR_OM_METAMAPALLOCATOR_HPP_

#include <OMR/Om/Allocator.hpp>
#include <OMR/Om/Context.hpp>
#include <OMR/Om/Initializer.hpp>
#include <OMR/Om/MetaMap.hpp>

namespace OMR {
namespace Om {

struct MetaMapInitializer : public Initializer {
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto m = reinterpret_cast<MetaMap*>(cell);
    m->baseMap().map(m);  // m describes its own shape.
    m->baseMap().kind(Map::Kind::META_MAP);
    return &m->baseCell();
  }
};

inline MetaMap* MetaMap::allocate(Context& cx) {
  MetaMapInitializer init;
  return BaseAllocator::allocate<MetaMap>(cx, init, sizeof(MetaMap));
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_METAMAPALLOCATOR_HPP_
