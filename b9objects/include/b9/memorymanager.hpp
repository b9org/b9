#if !defined(B9_MEMORYMANAGER_HPP_)
#define B9_MEMORYMANAGER_HPP_

#include <b9/rooting.hpp>
#include <b9/runtime.hpp>
#include <unordered_set>

namespace b9 {

class Cell;
class Map;
class MapMap;
class EmptyObjectMap;
class Object;
class Visitor;

class Context;
class RunContext;
class StartupContext;

struct Globals {
  MapMap* mapMap = nullptr;
  EmptyObjectMap* emptyObjectMap = nullptr;
};

using ContextSet = std::unordered_set<Context*>;

class MemoryManager {
 public:
  explicit MemoryManager(ProcessRuntime& runtime);

  MemoryManager(const MemoryManager&) = delete;

  ~MemoryManager();

  OMR_VM& omrVm() { return omrVm_; }

  const OMR_VM& omrVm() const { return omrVm_; }

  ProcessRuntime& runtime() { return runtime_; }

  const Globals& globals() const { return globals_; }

  MarkingFnVector userRoots() noexcept { return userRoots_; }

  const MarkingFnVector userRoots() const noexcept { return userRoots_; }

  void visitRoots(Context& cx, Visitor& visitor);

  const ContextSet& contexts() const { return contexts_; }

  ContextSet& contexts() { return contexts_; }

 protected:
  friend class Context;

 private:
  void initOmrVm();

  void initOmrGc();

  void initGlobals(StartupContext& cx);

  ProcessRuntime& runtime_;
  OMR_VM omrVm_;
  Globals globals_;
  MarkingFnVector userRoots_;
  ContextSet contexts_;
};

}  // namespace b9

#endif  // B9_MEMORYMANAGER_HPP_
