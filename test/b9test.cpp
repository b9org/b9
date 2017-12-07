#include <ObjectAllocationModel.hpp>
#include <b9/interpreter.hpp>
#include <b9/loader.hpp>
#include <b9/objects.hpp>
#include <b9/objects.inl.hpp>
// #include <omrgcallocate.hpp>
#include <b9/allocator.hpp>
#include <b9/context.hpp>
#include <b9/runtime.hpp>

#include <b9/allocator.inl.hpp>
#include <b9/memorymanager.inl.hpp>
#include <b9/rooting.inl.hpp>

#include <omrgc.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>

#include <gtest/gtest.h>

namespace b9 {
namespace test {

// clang-format off
const std::vector<const char*> TEST_NAMES = {
  "test_return_true",
  "test_return_false",
  "test_add",
  "test_sub",
  // CASCON2017 add test_div and test_mul here
  "test_equal",
  "test_equal_1",
  "test_greaterThan",
  "test_greaterThan_1",
  "test_greaterThanOrEqual",
  "test_greaterThanOrEqual_1",
  "test_greaterThanOrEqual_2",
  "test_lessThan",
  "test_lessThan_1",
  "test_lessThan_2",
  "test_lessThan_3",
  "test_lessThanOrEqual",
  "test_lessThanOrEqual_1",
  "test_call",
  "test_string_declare_string_var",
  "helper_test_string_return_string",
  "test_string_return_string",
  "test_while"
};
// clang-format on

ProcessRuntime runtime;

TEST(MemoryManagerTest, startUpAndShutDown) {
  MemoryManager manager(runtime);
  EXPECT_NE(manager.globals().metaMap, nullptr);
  EXPECT_NE(manager.globals().emptyObjectMap, nullptr);
  Context cx(manager);
  EXPECT_NE(cx.globals().metaMap, nullptr);
  EXPECT_NE(cx.globals().emptyObjectMap, nullptr);
  MetaMap* metaMap = allocateMetaMap(cx);
  EXPECT_EQ(metaMap, metaMap->map());
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
  Object* object = allocateEmptyObject(cx);
  EXPECT_EQ(object->map(), cx.globals().emptyObjectMap);
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  // EXPECT_EQ(object->map(), (Map*)0x5e5e5e5e5e5e5e5eul);
}

TEST(MemoryManagerTest, keepAnObject) {
  MemoryManager manager(runtime);
  Context cx(manager);
  RootRef<Object> object(cx, allocateEmptyObject(cx));
  EXPECT_EQ(object->map(), cx.globals().emptyObjectMap);
  OMR_GC_SystemCollect(cx.omrVmThread(), 0);
  EXPECT_EQ(object->map(), cx.globals().emptyObjectMap);
}

class InterpreterTest : public ::testing::Test {
 public:
  std::shared_ptr<Module> module_;

  virtual void SetUp() {
    auto moduleName = getenv("B9_TEST_MODULE");
    ASSERT_NE(moduleName, nullptr);
    module_ = DlLoader{}.loadModule(moduleName);
  }
};

TEST_F(InterpreterTest, interpreter) {
  Config cfg;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInteger()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit) {
  Config cfg;
  cfg.jit = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInteger()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_dc) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInteger()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_pp) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInteger()) << "Test Failed: " << test;
  }
}

TEST_F(InterpreterTest, jit_lvms) {
  Config cfg;
  cfg.jit = true;
  cfg.directCall = true;
  cfg.passParam = true;
  cfg.lazyVmState = true;

  VirtualMachine vm{runtime, cfg};
  vm.load(module_);
  vm.generateAllCode();

  for (auto test : TEST_NAMES) {
    EXPECT_TRUE(vm.run(test, {}).getInteger()) << "Test Failed: " << test;
  }
}

TEST(MyTest, arguments) {
  b9::VirtualMachine vm{runtime, {}};
  auto m = std::make_shared<Module>();
  Instruction i[] = {{ByteCode::PUSH_FROM_VAR, 0},
                     {ByteCode::PUSH_FROM_VAR, 1},
                     {ByteCode::INT_ADD},
                     {ByteCode::FUNCTION_RETURN},
                     END_SECTION};

  m->functions.push_back(b9::FunctionSpec{"add_args", i, 2, 0});
  vm.load(m);
  auto r = vm.run("add_args", {{Value::integer, 1}, {Value::integer, 2}});
  EXPECT_EQ(r, Value(Value::integer, 3));
}

extern "C" void b9_prim_print_string(ExecutionContext* context);

TEST(ObjectTest, allocateSomething) {
  b9::VirtualMachine vm{runtime, {}};
  auto m = std::make_shared<Module>();
  Instruction i[] = {
      {ByteCode::NEW_OBJECT},            // new object
      {ByteCode::POP_INTO_VAR, 0},       // store object into var0
      {ByteCode::STR_PUSH_CONSTANT, 0},  // push "voila"
      {ByteCode::PUSH_FROM_VAR, 0},      // push var0 aka object
      {ByteCode::POP_INTO_OBJECT, 0},    // pop "voila" into object at slot 0
      {ByteCode::SYSTEM_COLLECT},        // GC. Object is kept alive by var0
      {ByteCode::PUSH_FROM_VAR, 0},      // push object
      {ByteCode::PUSH_FROM_OBJECT, 0},   // get the string back
      {ByteCode::PRIMITIVE_CALL, 0},     // call b9_prim_print_string
      {ByteCode::FUNCTION_RETURN},       // finish with constant 0
      END_SECTION};
  m->strings.push_back("Hello, World");
  m->functions.push_back(b9::FunctionSpec{"allocate_object", i, 0, 1});
  m->primitives.push_back(b9_prim_print_string);
  vm.load(m);
  Value r = vm.run("allocate_object", {});
  EXPECT_EQ(r, Value(Value::integer, 0));
}

// clang-format off
std::vector<std::int32_t> integers = {
  0, 1, -1, 42, -42,
  std::numeric_limits<std::int32_t>::max(),
  std::numeric_limits<std::int32_t>::min()
};
// clang-format on

TEST(DoubleTest, canonicalNan) {
  EXPECT_TRUE(std::isnan(makeDouble(CANONICAL_NAN)));
}

TEST(ValueTest, integerConstructorRoundTrip) {
  for (auto i : integers) {
    Value value(Value::integer, i);
    auto i2 = value.getInteger();
    EXPECT_EQ(i, i2);
  }
}

TEST(ValueTest, setIntegerRoundTrip) {
  for (auto i : integers) {
    Value value;
    value.setInteger(i);
    auto i2 = value.getInteger();
    EXPECT_EQ(i, i2);
  }
}

TEST(ValueTest, canonicalNan) {
  EXPECT_EQ((CANONICAL_NAN & Double::SIGN_MASK), 0);
  EXPECT_NE((CANONICAL_NAN & BoxTag::MASK), BoxTag::VALUE);
  EXPECT_NE(makeDouble(CANONICAL_NAN), makeDouble(CANONICAL_NAN));
}

TEST(ValueTest, doubleRoundTrip) {
  const std::vector<double> doubles =  //
      {0.0,
       1.0,
       43.21,
       std::numeric_limits<double>::infinity(),
       std::numeric_limits<double>::max(),
       std::numeric_limits<double>::min()};

  for (auto d : doubles) {
    for (auto sign : {+1.0, -1.0}) {
      d *= sign;
      Value value;
      value.setDouble(d);
      EXPECT_EQ(d, value.getDouble());
      EXPECT_FALSE(value.isBoxedValue());
      EXPECT_TRUE(value.isDouble());
    }
  }
}

TEST(ValueTest, signalingNanDouble) {
  Value value;
  value.setDouble(std::numeric_limits<double>::signaling_NaN());
  EXPECT_TRUE(std::isnan(value.getDouble()));
  EXPECT_FALSE(value.isBoxedValue());
  EXPECT_NE(value.getDouble(), makeDouble(CANONICAL_NAN));
  EXPECT_EQ(value.raw(), CANONICAL_NAN);
}

TEST(ValueTest, quietNanDouble) {
  Value value;
  value.setDouble(std::numeric_limits<double>::quiet_NaN());
  EXPECT_TRUE(std::isnan(value.getDouble()));
  EXPECT_FALSE(value.isBoxedValue());
  EXPECT_NE(value.getDouble(), makeDouble(CANONICAL_NAN));
  EXPECT_EQ(value.raw(), CANONICAL_NAN);
}

TEST(ValueTest, pointerRoundTrip) {
  for (void* p : {(void*)0, (void*)1, (void*)(-1 & VALUE_MASK)}) {
    Value value;
    value.setPtr(p);
    EXPECT_EQ(p, value.getPtr());
    EXPECT_TRUE(value.isBoxedValue());
    EXPECT_TRUE(value.isPtr());
  }
}

}  // namespace test
}  // namespace b9
