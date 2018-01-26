
// #include <OMR/Om/Ref.hpp>

#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/Handle.hpp>
#include <OMR/Om/MemHandle.hpp>
#include <OMR/Om/MemoryManager.inl.hpp>
#include <OMR/Om/RootRef.inl.hpp>
#include <OMR/Om/Runtime.hpp>

#include <gtest/gtest.h>

namespace OMR {
namespace Om {
namespace Test {

ProcessRuntime runtime;

TEST(RootTest, basic) {
  MemoryManager manager(runtime);
  Context cx(manager);
  RootRef<Object> root(cx, allocateEmptyObject(cx));
  Handle<Object> handle(root);
  // MemHandle<Object::Base> member(handle, &Object::base);
}

}  // namespace Test
}  // namespace Om
}  // namespace OMR
