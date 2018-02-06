

#include <OMR/Infra/Span.hpp>
#include <OMR/Om/Allocation.hpp>
#include <OMR/Om/Allocator.inl.hpp>
#include <OMR/Om/ArrayBuffer.inl.hpp>
#include <OMR/Om/ArrayBufferMap.inl.hpp>
#include <OMR/Om/Context.hpp>
#include <OMR/Om/Map.inl.hpp>
#include <OMR/Om/MemoryManager.inl.hpp>
#include <OMR/Om/Object.inl.hpp>
#include <OMR/Om/ObjectMap.inl.hpp>
#include <OMR/Om/RootRef.inl.hpp>
#include <OMR/Om/Runtime.hpp>

#include <omrgc.h>

#include <gtest/gtest.h>

namespace OMR {
namespace Om {
namespace Test {

ProcessRuntime runtime;

TEST(MemoryManagerTest, StartUpAMemoryManager) {
  MemoryManager manager(runtime);
  EXPECT_NE(manager.globals().metaMap(), nullptr);
  EXPECT_NE(manager.globals().arrayBufferMap(), nullptr);
}

TEST(MemoryManagerTest, StartUpAContext) {
  MemoryManager manager(runtime);
  Context cx(manager);
  EXPECT_NE(cx.globals().metaMap(), nullptr);
  EXPECT_NE(cx.globals().arrayBufferMap(), nullptr);
}

TEST(MemoryManagerTest, AllocateAMetaMap) {
  MemoryManager manager(runtime);
  Context cx(manager);
  MetaMap* metaMap = MetaMap::allocate(cx);
  EXPECT_EQ(metaMap, metaMap->map());
}

TEST(MemoryManagerTest, loseAnObjects) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<ObjectMap> map(cx, ObjectMap::allocate(cx));
  Object* object = Object::allocate(cx, map);

  EXPECT_NE(object->map(), nullptr);
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  // EXPECT_EQ(object->map(), (Map*)0x5e5e5e5e5e5e5e5eul);
}

TEST(MemoryManagerTest, keepAnObject) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<ObjectMap> map(cx, ObjectMap::allocate(cx));
  RootRef<Object> object(cx, Object::allocate(cx, map));
  EXPECT_EQ(object->map(), map.get());
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  EXPECT_EQ(object->map(), map.get());
}

TEST(MemoryManagerTest, objectTransition) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<ObjectMap> emptyObjectMap(cx, ObjectMap::allocate(cx));
  RootRef<Object> obj1(cx, Object::allocate(cx, emptyObjectMap));

  const std::array<const SlotDescriptor, 1> descriptors{
      {SlotDescriptor(SlotType(Id(0), CoreType::VALUE), Id(0))}};

  const Infra::Span<const Om::SlotDescriptor> span(descriptors);
  Object::transition(cx, obj1, span, Om::hash(span));

  // check
  {
    auto m = obj1->map();
    EXPECT_EQ(m->baseCell().map(), &cx.globals().metaMap()->baseMap());
    EXPECT_EQ(m->slotDescriptors(), descriptors);
    EXPECT_EQ(m->slotCount(), 1);
    EXPECT_EQ(m->parent()->slotOffset(), 0);
  }

  // second object
  {
    RootRef<Object> obj2(cx, Object::allocate(cx, emptyObjectMap));
    auto m =
        obj2->takeExistingTransition(cx, descriptors, Om::hash(descriptors));
    EXPECT_NE(m, nullptr);
    EXPECT_EQ(m, obj1->map());
    EXPECT_EQ(m, obj2->map());
  }
}

TEST(MemoryManagerTest, objectTransitionReuse) {
  MemoryManager manager(runtime);
  Context cx(manager);
}

}  // namespace Test
}  // namespace Om
}  // namespace OMR
