
#if !defined(OMR_OM_EMPTYOBJECTMAP_INL_HPP_)
#define OMR_OM_EMPTYOBJECTMAP_INL_HPP_

#include <OMR/Om/Allocator.hpp>
#include <OMR/Om/Context.hpp>
#include <OMR/Om/EmptyObjectMap.hpp>
#include <OMR/Om/Handle.hpp>
#include <OMR/Om/Initializer.hpp>

namespace OMR {
namespace Om {

struct EmptyObjectMapInitializer : public Initializer {
 public:
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto m = reinterpret_cast<EmptyObjectMap*>(cell);
    m->baseMap().map(cx.globals().metaMap());
    m->baseMap().kind(Map::Kind::EMPTY_OBJECT_MAP);
    return &m->baseCell();
  }
};

inline EmptyObjectMap* EmptyObjectMap::allocate(Context& cx) {
  EmptyObjectMapInitializer init;
  RootRef<EmptyObjectMap> root(
      cx, BaseAllocator::allocate<EmptyObjectMap>(cx, init));
  bool ok = EmptyObjectMap::construct(cx, root);

  if (!ok) return nullptr;

  return root.get();
}

inline bool EmptyObjectMap::construct(Context& cx,
                                      Handle<EmptyObjectMap> self) {
  return ObjectMap::construct(cx, self.reinterpret<ObjectMap>());
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_EMPTYOBJECTMAP_INL_HPP_
