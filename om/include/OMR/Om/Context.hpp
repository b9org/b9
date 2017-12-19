#if !defined(OMR_OM_CONTEXT_HPP_)
#define OMR_OM_CONTEXT_HPP_

#include <OMR/Om/MemoryManager.hpp>
#include <OMR/Om/RootRef.hpp>
#include <OMR/Om/Runtime.hpp>

#include <mminitcore.h>
#include <omrutil.h>
#include <OMR_VMThread.hpp>

#include <cstddef>
#include <cstdint>
#include <new>
#include <unordered_set>

class MM_EnvironmentBase;

namespace OMR {
namespace Om {

class Cell;
class Map;
class MetaMap;
class EmptyObjectMap;
class ObjectMap;
class Object;
class RefSeq;

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

  MarkingFnVector& userRoots() noexcept { return userRoots_; }

  const MarkingFnVector& userRoots() const noexcept { return userRoots_; }

 private:
  MemoryManager& manager_;
  OMR_VMThread* omrVmThread_;
  RootRefSeq stackRoots_;
  MarkingFnVector userRoots_;
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
 public:
  RunContext(MemoryManager& manager) : Context(manager) {}
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_CONTEXT_HPP_
