#if !defined(B9_CONTEXT_HPP_)
#define B9_CONTEXT_HPP_

#include <mminitcore.h>
#include <omrutil.h>
#include <OMR_VMThread.hpp>
#include <b9/memorymanager.hpp>
#include <b9/runtime.hpp>
#include <b9/rooting.hpp>
#include <cstddef>
#include <cstdint>
#include <new>

class MM_EnvironmentBase;

namespace b9 {

class Cell;
class Map;
class MapMap;
class EmptyObjectMap;
class ObjectMap;
class Object;
class RootRefSeq;

/// A GC context.
class Context {
 public:
  static constexpr const char* THREAD_NAME = "b9_context";

  Context(MemoryManager& manager);

  Context(const Context& other) = delete;

  ~Context() noexcept;

  MemoryManager& manager() const noexcept { return manager_; }

  const Globals& globals() const noexcept { return manager().globals(); }

  OMR_VMThread* omrVmThread() const noexcept { return omrVmThread_; }

  MM_EnvironmentBase* omrGcThread() const noexcept;

  RootRefSeq& stackRoots() noexcept { return stackRoots_; }

 private:
  MemoryManager& manager_;
  OMR_VMThread* omrVmThread_;
  RootRefSeq stackRoots_;
};

inline Context& getContext(OMR_VMThread& omrVmThread) {
	return *(Context*)omrVmThread._language_vmthread;
}

inline Context& getContext(OMR_VMThread* omrVmThread) {
	return getContext(*omrVmThread);
}

/// A special limited context that is only used during startup or shutdown.
class StartupContext : public Context {
 protected:
  friend class MemoryManager;
  StartupContext(MemoryManager& manager) : Context(manager) {}

  StartupContext(const StartupContext& other) = delete;
};

/// A full runtime context.
class RunContext : public Context {
  RunContext(MemoryManager& manager) : Context(manager) {}
};

}  // namespace b9

#endif  // B9_CONTEXT_HPP_
