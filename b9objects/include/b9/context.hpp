#if !defined(B9_CONTEXT_HPP_)
#define B9_CONTEXT_HPP_

#include <mminitcore.h>
#include <omrutil.h>
#include <OMR_VMThread.hpp>
#include <b9/runtime.hpp>
#include <cstddef>
#include <cstdint>
#include <new>

class Cell;
class Map;
class MapMap;
class EmptyObjectMap;
class ObjectMap;
class Object;

namespace b9 {

class Context {
 public:
  static constexpr const char* THREAD_NAME = "b9_context";

  Context(MemoryManager& manager) : manager_(manager) {
    auto e =
        OMR_Thread_Init(&manager.omrVm(), this, &omrVmThread_, "b9::Context");
    if (e != 0)
      throw std::runtime_error("Failed to attach OMR thread to OMR VM");
  }

  ~Context() noexcept {
    OMR_Thread_Free(omrVmThread_);
    // omrVmThread_ = nullptr_t;
  }

  MemoryManager& manager() const noexcept { return manager_; }

  OMR_VMThread* omrVmThread() const noexcept { return omrVmThread_; }

 private:
  MemoryManager& manager_;
  OMR_VMThread* omrVmThread_;
};

struct RawAllocator {};

struct Allocator {
#if 0
  template <typename T, typename... Args>
  T* allocate(Context& cx, Args&&... args) {
    auto p = rawAllocator.allocate(sizeof(T));

    /// Some sneaky notes: The constructor for T should not sub-allocate. It
    /// must do the minimum initialization to make the object walkable, in the
    /// case of a concurrent GC scan.
    new (p)(std::forward<Args>(args)...);

    cx->saveStack().push(p);
    pay_tax(cx);
    cx->saveStack.pop();
  }
#endif
};

#if 0
struct GlobalContext {
 public:
  MapMap* mapMap() const { return mapMap_; }

  EmptyObjectMap* emptyObjectMap() const { return emptyObjectMap_; }

 private:
  MapMap* mapMap_;
  EmptyObjectMap* emptyObjectMap_;
};

struct Context {
 public:
  Allocator& allocator() { return allocator_; }


 private:
  Allocator allocator_;
  // std::stack<Cell*> saveStack_;
};

struct ContextState {
};

};

//
// Context and Byte Allocator
//




#endif  // 0

}  // namespace b9

#endif  // B9_CONTEXT_HPP_
