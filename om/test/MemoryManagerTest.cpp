

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
#include <OMR/Om/SlotMap.inl.hpp>

#include <omrgc.h>

#include <gtest/gtest.h>

namespace OMR {
namespace Om {
namespace Test {

ProcessRuntime runtime;

TEST(MemoryManagerTest, startUpAndShutDown) {
  MemoryManager manager(runtime);
  EXPECT_NE(manager.globals().metaMap(), nullptr);
  EXPECT_NE(manager.globals().emptyObjectMap(), nullptr);
  Context cx(manager);
  EXPECT_NE(cx.globals().metaMap(), nullptr);
  EXPECT_NE(cx.globals().emptyObjectMap(), nullptr);
  MetaMap* metaMap = MetaMap::allocate(cx);
  EXPECT_EQ(&metaMap->baseMap(), metaMap->baseCell().map());
}

TEST(MemoryManagerTest, startUpAContext) {
  MemoryManager manager(runtime);
  Context cx(manager);
  EXPECT_NE(cx.globals().metaMap(), nullptr);
  EXPECT_NE(cx.globals().emptyObjectMap(), nullptr);
}

TEST(MemoryManagerTest, allocateTheMetaMap) {
  MemoryManager manager(runtime);
  Context cx(manager);
  MetaMap* metaMap = MetaMap::allocate(cx);
  EXPECT_EQ(metaMap, metaMap->map());
}

TEST(MemoryManagerTest, loseAnObjects) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<EmptyObjectMap> map(cx, EmptyObjectMap::allocate(cx));
  Object* object = Object::allocate(cx, map);

  EXPECT_NE(object->map(), nullptr);
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  // EXPECT_EQ(object->map(), (Map*)0x5e5e5e5e5e5e5e5eul);
}

TEST(MemoryManagerTest, keepAnObject) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<EmptyObjectMap> map(cx, EmptyObjectMap::allocate(cx));
  RootRef<Object> object(cx, Object::allocate(cx, map));
  EXPECT_EQ(object->map(), &map->baseObjectMap());
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  EXPECT_EQ(object->map(), &map->baseObjectMap());
}

TEST(MemoryManagerTest, objectTransition) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<EmptyObjectMap> emptyObjectMap(cx, EmptyObjectMap::allocate(cx));
  RootRef<Object> obj1(cx, Object::allocate(cx, emptyObjectMap));

  SlotDescriptor slotd(SlotType(Id(0), CoreType::VALUE), Id(42));
  Object::transition(cx, obj1, slotd, slotd.hash());

  // check
  {
    auto m = reinterpret_cast<SlotMap*>(obj1->map());
    EXPECT_EQ(m->baseMap().map(), cx.globals().metaMap());
    EXPECT_EQ(m->slotDescriptor(), slotd);
    EXPECT_EQ(m->index(), 0);
    EXPECT_EQ(m->kind(), Map::Kind::SLOT_MAP);
    EXPECT_EQ(m->parent()->kind(), Map::Kind::EMPTY_OBJECT_MAP);
  }

  // second object
  {
    RootRef<Object> obj2(cx, Object::allocate(cx, emptyObjectMap));
    auto m = obj2->takeExistingTransition(cx, slotd, slotd.hash());
    EXPECT_NE(m, nullptr);
    EXPECT_EQ(&m->baseObjectMap(), obj1->map());
    EXPECT_EQ(&m->baseObjectMap(), obj2->map());
    EXPECT_EQ(obj2->map(), obj1->map());
  }
}

TEST(MemoryManagerTest, objectTransitionReuse) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<EmptyObjectMap> base_r(cx, EmptyObjectMap::allocate(cx));
  RootRef<Object> obj_r(cx, Object::allocate(cx, base_r));
}

}  // namespace Test
}  // namespace Om
}  // namespace OMR
