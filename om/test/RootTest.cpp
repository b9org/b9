
// #include <OMR/Om/Ref.hpp>

#include <OMR/Om/Runtime.hpp>
#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/MemoryManager.inl.hpp>
#include <OMR/Om/RootRef.inl.hpp>
#include <OMR/Om/Handle.hpp>
#include <OMR/Om/MemHandle.hpp>

#include <gtest/gtest.h>

namespace OMR {
namespace Om {
namespace Test {

ProcessRuntime runtime;

TEST(RootTest, basic) {
	MemoryManager manager;
	Context cx(manager);
	RootRef<Object> root(cx, allocateEmptyObject(cx));
	Handle<Object> handle(root);
	MemberHandle<CellHeader> member(handle, &Object::)
}

}  // namespace Test
}  // namespace Om
}  // namespace OMR