#if !defined(B9_ALLOCATOR_HPP_)
#define B9_ALLOCATOR_HPP_

#include <ObjectAllocationModel.hpp>
#include <b9/context.hpp>
#include <b9/runtime.hpp>
#include <b9/objects.inl.hpp>
#include <b9/objects.hpp>
#include <omrgc.h>

namespace b9 {

struct MapMapInitializer : public Initializer {
	virtual Cell* operator()(Cell* cell) override {
		return new(cell) MapMap();
	}
};

inline MapMap* allocateMapMap(Context& cx) {
	MapMapInitializer init;
	Allocation allocation(cx, init, sizeof(MapMap));
	return (MapMap*) OMR_GC_AllocateObject(cx.omrVmThread(), &allocation);
}

inline Object* allocateEmptyObject(Context& cx) {
  EmptyObjectMap map(nullptr); // TODO: Get EmptyObjectMap off cx.
  EmptyObjectInitializer init;
  init.map = &map;
  Allocation allocation(cx, init, sizeof(Object));
  return (Object*) OMR_GC_AllocateObject(cx.omrVmThread(), &allocation);
}

} // namespace b9

#endif // B9_ALLOCATOR_HPP_
