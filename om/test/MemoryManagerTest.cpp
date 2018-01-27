

#include <OMR/Om/Allocation.hpp>
#include <OMR/Om/Allocator.inl.hpp>
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
  EXPECT_NE(manager.globals().metaMap, nullptr);
  EXPECT_NE(manager.globals().emptyObjectMap, nullptr);
  Context cx(manager);
  EXPECT_NE(cx.globals().metaMap, nullptr);
  EXPECT_NE(cx.globals().emptyObjectMap, nullptr);
  MetaMap* metaMap = allocateMetaMap(cx);
  EXPECT_EQ(&metaMap->baseMap(), metaMap->baseCell().map());
}

TEST(MemoryManagerTest, startUpAContext) {
  MemoryManager manager(runtime);
  Context cx(manager);
  EXPECT_NE(cx.globals().metaMap, nullptr);
  EXPECT_NE(cx.globals().emptyObjectMap, nullptr);
}

TEST(MemoryManagerTest, allocateTheMetaMap) {
  MemoryManager manager(runtime);
  Context cx(manager);
  MetaMap* metaMap = allocateMetaMap(cx);
  EXPECT_EQ(metaMap, metaMap->map());
}

TEST(MemoryManagerTest, loseAnObjects) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<EmptyObjectMap> map(cx, allocateEmptyObjectMap(cx));
  Object* object = allocateObject(cx, map);

  EXPECT_NE(object->map(), nullptr);
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  // EXPECT_EQ(object->map(), (Map*)0x5e5e5e5e5e5e5e5eul);
}

TEST(MemoryManagerTest, keepAnObject) {
  MemoryManager manager(runtime);
  Context cx(manager);

  RootRef<EmptyObjectMap> map(cx, allocateEmptyObjectMap(cx));
  RootRef<Object> object(cx, allocateObject(cx, map));
  EXPECT_EQ(object->map(), &map->baseObjectMap());
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  EXPECT_EQ(object->map(), &map->baseObjectMap());
}

TEST(MemoryManagerTest, objectTransition) {
  MemoryManager manager(runtime);
  Context cx(manager);

  Object* obj = nullptr;

  // allocate the object
  {
    RootRef<EmptyObjectMap> map(cx, allocateEmptyObjectMap(cx));
    obj = allocateObject(cx, map);
  }

  SlotDescriptor slotd(Id(42));

  // transition
  {
    RootRef<Object> obj_r(cx, obj);
    Object::transition(cx, obj_r, slotd, slotd.hash());
    obj = obj_r.get();
  }

  // check
  {
    auto map = reinterpret_cast<SlotMap*>(obj->map());
    EXPECT_EQ(map->baseMap().map(), cx.globals().metaMap);
    EXPECT_EQ(map->slotDescriptor(), slotd);
    EXPECT_EQ(map->index(), 0);
    EXPECT_EQ(map->kind(), Map::Kind::SLOT_MAP);
    EXPECT_EQ(map->parent()->kind(), Map::Kind::EMPTY_OBJECT_MAP);
  }
}

}  // namespace Test
}  // namespace Om
}  // namespace OMR
